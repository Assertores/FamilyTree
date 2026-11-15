# Handover
Notes from writing the documentation that matter for implementing the source code and the tests, but do not fit into any of the other documents. Entries link to related documents where they exist.

## Implementation Order
<!-- The order in which features and components should be built, and what depends on what. -->
<!-- TEMPLATE
1. STEP, and why it comes at this point
-->

## Pitfalls
<!-- Things that are easy to get wrong when implementing, e.g. subtle edge cases, misleading names, or behavior that looks like a bug but is intended. -->
<!-- TEMPLATE
- PITFALL
-->
- The original implementation in `original_implimentation/` assigns [Generations](../glossary.md#generation) by walking down from every Person without Parents, pushing each Child below its Parents, and then moving all Parents of a Child to the same Generation. Moving a Parent down can require moving their own ancestors and other descendants as well, so a single pass from the top is not enough. See [REQ 0045](../requirements/req_0045.md).
- The [Distance](../glossary.md#distance) of the [Family Tree](../glossary.md#family-tree) is counted on a graph in which every Family is a node and every Person is an edge between the Families they are a member of; a Person who is a Parent in several Families connects all of them. Starting from the Families of the selected Person at Distance 0, the walk goes from Family to Family, and collects all members of every Family it reaches. The earlier attempt counted Relations from Person to Person, which tore Families apart and showed Children with only one Parent.
- [Partners](../glossary.md#partner) come only from common Children. An Unrestricted Relation, e.g. a marriage, does not make two Persons Partners, because it can also describe a best friend.
- The Format Version of the existing data is the UUID `3B5589D2-D9AF-40A8-BC40-574DAB6FFC57`. Files with any other Format Version, or without one, are skipped.

## Testing Notes
<!-- Details needed for writing tests that the scenarios do not cover, e.g. required test data, hard to reproduce states, or external systems that need to be mocked. -->
<!-- TEMPLATE
- NOTE
-->
- The command line interface of the original implementation was a development tool, and is the intended driver for automated end-to-end tests.
- The example data in `original_implimentation/data/` is useful test data, with these caveats:
    - `1_Philippa_Perry/data.json` writes its date of death under the unknown key `death` instead of `DateOfDeath`. It is a ready-made case for [REQ 0005](../requirements/req_0005.md): the date of death has to come back as missing, and the key `death` has to be reported to the Trace.
    - `relations_extra.csv` and `4_Constance_Houghton/data_extra.json` are leftovers of an attempt to add data to a Data Set. They are not valid data and must not be used as examples of how several Data Folders are combined ([OQ 0006](../open_questions.md)).
    - `relations.csv` contains a row with an empty Relationship (`10,11,StrictlyLower,,,`), which tests the optional columns of [REQ 0007](../requirements/req_0007.md).
    - `macro.txt` is a command script for the original command line interface that loads the whole example data.

## Other
<!-- Anything else worth knowing before starting the implementation. -->
<!-- TEMPLATE
- NOTE
-->
- The Trace viewer of the original implementation is a development tool only. It may become unnecessary if the [Trace](../glossary.md#trace) is written in a better readable format.
- `partial_rewrite/` contains a started rewrite of the Library: the program is asked step by step for folder lists, file contents and parsing, and can answer itself or ask for the Library's default; the separate data provider interfaces are gone; text is UTF-8; search returns Person Ids; and merging two Contexts is planned. It is not fully working.
- Left to the implementation, to be decided with the user when the work package comes up:
    - whether a search ignores upper and lower case, e.g. whether "anna" finds "Anna" ([REQ 0016](../requirements/req_0016.md));
    - which controls the story player needs beyond pausing and continuing, e.g. jumping forward and back or a position bar, and whether a Recording keeps playing while the user moves to another Person in the Family Tree ([REQ 0012](../requirements/req_0012.md)).
- Combining several Data Folders, e.g. to keep sensitive Details in a separate Data Folder, is a stretch goal, and how it works is still open. Worth keeping in mind when designing the Library: once Data Folders are combined, the Person Id of a Person File may no longer be unique ([OQ 0006 to 0008](../open_questions.md)).
