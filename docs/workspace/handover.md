# Handover
Notes from writing the documentation that matter for implementing the source code and the tests, but do not fit into any of the other documents. Entries link to related documents where they exist.

## Implementation Order
<!-- The order in which features and components should be built, and what depends on what. -->
<!-- TEMPLATE
1. STEP, and why it comes at this point
-->
1. Build setup, `.clang-tidy` for the Library, compiler flags, sanitizers, and the test framework `check.hpp` carried over from `partial_rewrite/lib_family_tree/tests/src/check.hpp` into `tests/framework/` ([ADR 0014](../decisions/adr_0014.md)), because every later step relies on them catching mistakes ([ADR 0011](../decisions/adr_0011.md)).
2. Trace writer and `FT_Result`, because every other function of the Library reports through them ([ADR 0007](../decisions/adr_0007.md), [ADR 0008](../decisions/adr_0008.md)).
3. JSON and CSV parsers, because the Builder needs them, and they can be tested on strings alone.
4. Builder with its Questions and the Default Platform, because it is the only way into a Context ([ADR 0005](../decisions/adr_0005.md)).
5. Resolver: Parents, Children, Siblings, Partners, Families and Generations, including the detection of conflicting Generations ([REQ 0045](../requirements/req_0045.md), [REQ 0047](../requirements/req_0047.md)).
6. Queries, including the search and the Family Tree query ([ADR 0004](../decisions/adr_0004.md), [ADR 0009](../decisions/adr_0009.md)).
7. Command Line Application, because the end-to-end tests of every feature need it ([ADR 0012](../decisions/adr_0012.md)).
8. Data Entry Tool, before the Browsing Application, because the data collector can start recording the family with it while the rest is built.
9. Browsing Application, after [SPK 0001](../spikes/spk_0001.md) has settled the layout of the Family Tree.

## Pitfalls
<!-- Things that are easy to get wrong when implementing, e.g. subtle edge cases, misleading names, or behavior that looks like a bug but is intended. -->
<!-- TEMPLATE
- PITFALL
-->
- The original implementation in `original_implimentation/` assigns [Generations](../glossary.md#generation) by walking down from every Person without Parents, pushing each Child below its Parents, and then moving all Parents of a Child to the same Generation. Moving a Parent down can require moving their own ancestors and other descendants as well, so a single pass from the top is not enough. See [REQ 0045](../requirements/req_0045.md).
- The original implementation does not detect conflicting Generations and recurses until it runs out of stack. Two cases have to be detected ([REQ 0047](../requirements/req_0047.md)): a Person who is their own ancestor, and two Parents of the same Child where one is an ancestor of the other. When all Parents of a Child are merged into one node, both cases are a cycle. The recursion depth of the original also grows with the number of Generations, so an iterative algorithm is safer.
- Every public function of the original implementation starts with the macro `SET_TRACE` in `original_implimentation/lib_family_tree/src/context.c`: if the Application passed no Trace, or one with a missing function pointer, it creates a no-op Trace. Lower level functions can then rely on a Trace being there and never check for it ([REQ 0044](../requirements/req_0044.md)).
- The [Distance](../glossary.md#distance) of the [Family Tree](../glossary.md#family-tree) is counted on a graph in which every Family is a node and every Person is an edge between the Families they are a member of; a Person who is a Parent in several Families connects all of them. Starting from the Families of the selected Person at Distance 0, the walk goes from Family to Family, and collects all members of every Family it reaches. The earlier attempt counted Relations from Person to Person, which tore Families apart and showed Children with only one Parent.
- [Partners](../glossary.md#partner) come only from common Children. An Unrestricted Relation, e.g. a marriage, does not make two Persons Partners, because it can also describe a best friend.
- The Format Version of the existing data is the UUID `3B5589D2-D9AF-40A8-BC40-574DAB6FFC57`. It is replaced by a new Format Version in which "Remarks" is an array with one string per line ([REQ 0003](../requirements/req_0003.md)); its UUID has not been chosen yet. The Library only supports the new one. The Person Files the data collector has written so far are converted once, by hand or with a small script: the remark split into lines, and the new UUID.
- The original JSON parser does not decode escape sequences: it reads from one quotation mark to the next. A `\n` came back as a backslash and an `n`, and a `\"` ended the string early and broke the rest of the file. The new parser decodes `\"`, `\\`, `\/`, `\b`, `\f`, `\n`, `\r`, `\t` and `\uXXXX`, including surrogate pairs, into UTF-8 ([REQ 0009](../requirements/req_0009.md)).

- The answers to Questions point to memory of the Application. The Library copies what it needs before `FT_Continue` returns ([ADR 0005](../decisions/adr_0005.md)); keeping a pointer into an answer is a use after free waiting to happen.
- Views are `const` pointers into the Context ([ADR 0004](../decisions/adr_0004.md)). Nothing may write through them, and the Context may not be reallocated after creation, or every view an Application holds becomes invalid.
- The Library uses the Person Id of the Person File everywhere, and never assigns ids of its own, unlike the original implementation, which gave every Person a new id. Person Ids need not be consecutive, so looking a Person up needs an index built at creation, e.g. the Persons sorted by Person Id and a binary search ([ADR 0013](../decisions/adr_0013.md)).
- Qt has to be linked dynamically because of its LGPL license ([DEP 0001](../dependencies/dep_0001.md)); the Library itself may be linked statically.

## Testing Notes
<!-- Details needed for writing tests that the scenarios do not cover, e.g. required test data, hard to reproduce states, or external systems that need to be mocked. -->
<!-- TEMPLATE
- NOTE
-->
- The command line interface of the original implementation was a development tool, and is the intended driver for automated end-to-end tests.
- The example data in `original_implimentation/data/` is useful test data, with these caveats:
    - `1_Philippa_Perry/data.json` writes its date of death under the unknown key `death` instead of `DateOfDeath`. It is a ready-made case for [REQ 0005](../requirements/req_0005.md): the date of death has to come back as missing, and the key `death` has to be reported to the Trace.
    - `relations_extra.csv` and `4_Constance_Houghton/data_extra.json` are leftovers of an attempt to add data to a Data Set. They are not valid data and must not be used as examples of how several Data Folders are combined ([OQ 0006](../open_questions.md)).
    - `relations.csv` has no Format Version line yet ([REQ 0006](../requirements/req_0006.md)). It needs the line `Version,<UUID>` in front of its header before it is valid test data. The UUID of the first Format Version of the Relations File has not been chosen yet; it is a new UUID, not the one of the Person File.
    - `relations.csv` contains a row with an empty Relationship (`10,11,StrictlyLower,,,`), which tests the optional columns of [REQ 0007](../requirements/req_0007.md).
    - `macro.txt` is a command script for the original command line interface that loads the whole example data.

## Other
<!-- Anything else worth knowing before starting the implementation. -->
<!-- TEMPLATE
- NOTE
-->
- The Trace viewer of the original implementation is a development tool only. It may become unnecessary if the [Trace](../glossary.md#trace) is written in a better readable format.
- `partial_rewrite/` contains a started rewrite of the Library: the program is asked step by step for folder lists, file contents and parsing, and can answer itself or ask for the Library's default; the separate data provider interfaces are gone; text is UTF-8; search returns Person Ids; and merging two Contexts is planned. It is not fully working.
- The feature records [FTR 0001](../features/ftr_0001.md) to [FTR 0009](../features/ftr_0009.md) were written during step 2, before the architecture. Their scenarios cover the normal cases only. Scenarios for errors, e.g. a Data Folder that cannot be opened because of conflicting Generations, or a missing Media file, are missing on purpose: what the user sees depends on how steps, warnings and errors are told apart ([OQ 0019](../open_questions.md)). Add them once the architecture has decided it.
- Left to the implementation, to be decided with the user when the work package comes up:
    - whether a search ignores upper and lower case, e.g. whether "anna" finds "Anna" ([REQ 0016](../requirements/req_0016.md));
    - which controls the story player needs beyond pausing and continuing, e.g. jumping forward and back or a position bar, and whether a Recording keeps playing while the user moves to another Person in the Family Tree ([REQ 0012](../requirements/req_0012.md));
    - what the picture carousel does after the last Picture, e.g. start again with the first one ([REQ 0048](../requirements/req_0048.md)).
- Combining several Data Folders, e.g. to keep sensitive Details in a separate Data Folder, is a stretch goal, and how it works is still open. Worth keeping in mind when designing the Library: once Data Folders are combined, the Person Id of a Person File may no longer be unique ([OQ 0006 to 0008](../open_questions.md)).
- Deployment: the data collector uses Windows, the author develops on Linux. The Windows build of the Applications is made by continuous integration on a Windows runner ([ADR 0015](../decisions/adr_0015.md); the original implementation already had `.github/workflows/cmake-multi-platform.yml`), and downloaded as an artifact, which also checks that everything builds on Windows ([REQ 0038](../requirements/req_0038.md)). After building, `windeployqt` copies every Qt library and plugin the Applications need next to them: the platform plugin, the image formats for the picture carousel and the multimedia backend for the story player. The result is a folder, or an installer made from it.
- The German Qt translations, e.g. `qtbase_de.qm`, have to be bundled as well; otherwise the texts Qt brings itself, e.g. the buttons of standard dialogs, stay English ([REQ 0046](../requirements/req_0046.md)).
- Passing on the Applications with Qt is allowed by the LGPL, as long as ([DEP 0001](../dependencies/dep_0001.md)): the Qt libraries stay separate files, the license texts of the LGPL and GPL are included, the Applications say that they use Qt under the LGPL, e.g. in an About dialog, and the source code of the Qt version used stays available, e.g. as a kept copy of its source archive.
