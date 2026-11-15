# Implementer's Notes
What everyone implementing a part of the source code should know, written after the tests were implemented, followed by a log of the implementation sessions.

## Getting Started
<!-- How to build the project, run all tests, and run a single test. Where the entry points of the source code and the tests are. -->
### Tools
clang and clang++, clang-tidy, clang-format, CMake 3.20 or newer, Ninja, Doxygen, and Qt 6 for the Applications. Verified on Linux with clang, CMake 4.5 and Doxygen; Windows is verified by continuous integration once the package [Continuous Integration](#continuous-integration) is done.

### Commands
Configure and build everything, from the root of the repository. clang-tidy runs while building, so a warning fails the build.
```
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build
```

Run all tests: the Library tests `FamilyTreeTests`, the Data Entry Tool tests `DataEntryTests`, and the formatting check `Formatting`.
```
ctest --test-dir build --output-on-failure
```

Run a single test executable, either through CTest or directly, which prints every test with `[SUCCESS]` or `[ FAILED]` and the values that differ:
```
ctest --test-dir build -R DataEntryTests --output-on-failure
build/library/tests/FamilyTreeTests
build/applications/data_entry/DataEntryTests
```

A single test function cannot be selected yet: `check.hpp` has no filter. Until the package [Continuous Integration](#continuous-integration) adds one, comment out the other suites in `library/tests/main.cpp` while working, and restore them before the package is done.

Further targets and options:
```
cmake --build build --target documentation
cmake --build build --target format
cmake --build build --target check-format
cmake -S . -B build -DFT_CLANG_TIDY=OFF
cmake -S . -B build -DFT_BUILD_APPLICATIONS=OFF
```
- `documentation` runs Doxygen over the public header and fails on any undocumented function or parameter; it passes today.
- `FT_CLANG_TIDY=OFF` makes builds faster while experimenting; a package is only done when it builds with clang-tidy on.
- `FT_BUILD_APPLICATIONS=OFF` builds the Library and its tests without Qt.

The Library must also build without CMake, with a single compiler call ([REQ 0039](../requirements/req_0039.md)). This works today with gcc and clang, and has to keep working, so no source file may depend on a define or a generated file from CMake:
```
cc -std=c99 -pedantic-errors -Wall -Wextra -Werror -I library/include -c library/src/*.c
```

### State of the Tests
Before the first package, all 110 tests of the Library fail, and 10 of the 12 tests of the Data Entry Tool. Every failure comes from the stubs, which return `FT_ERROR_NOT_SUPPORTED` (the value 9 in the test output); no test fails because of its setup, its data or a crash, and the sanitizers report nothing. `Formatting` passes. The two Data Entry tests that pass, `RefusesAPersonFileOfAnotherFormatVersion` and `RefusesAMalformedPersonFile`, only pass because the stub refuses every file.

### Entry Points
| Part | Where |
| ---: | :--- |
| Public API | [library/include/family_tree/api.h](../../library/include/family_tree/api.h), complete and documented; its declarations do not change during the implementation |
| Library stubs | [builder.c](../../library/src/builder.c) creation, [context.c](../../library/src/context.c) `FT_Free`, [queries.c](../../library/src/queries.c) all queries and values, [default_platform.c](../../library/src/default_platform.c) the Media defaults; new source files go into `library/src/` and `library/CMakeLists.txt` |
| Library tests | [library/tests/](../../library/tests/), one file per topic, run by [main.cpp](../../library/tests/main.cpp) |
| Test helpers | [support/test_support.hpp](../../library/tests/support/test_support.hpp): the in-memory `DataFolder` that answers the Questions, `TraceBuffer`, `ContextGuard`, `PersonFile` and `RelationsFile` |
| Test framework | [tests/framework/check.hpp](../../tests/framework/check.hpp) |
| Data Entry Tool | [person_file_document.hpp](../../applications/data_entry/src/person_file_document.hpp) and its stub [person_file_document.cpp](../../applications/data_entry/src/person_file_document.cpp); tests in [applications/data_entry/tests/](../../applications/data_entry/tests/) |
| Example data | [tests/data/example/](../../tests/data/example/), the 18 Persons of the original implementation in the current Format Versions, each covering other edge cases, listed in the [handover](handover.md#testing-notes); the reference implementations themselves were removed by the package [Migration](#migration), see [General Notes](#general-notes) |
| Templates | [templates/](../../templates/), a Person File and a Relations File to fill in by hand |

## Work Packages
<!-- The implementation split into parts that can be done independently, what each part covers, and what each part depends on. -->
| Work Package | Scope | Tests | Depends On | State |
| ---: | :--- | :--- | :--- | :--- |
| Migration | move what is still needed out of `original_implimentation/` and `partial_rewrite/`: the example data, converted to the current Format Versions ([ADR 0016](../decisions/adr_0016.md)), the license, and the workflows ([ADR 0015](../decisions/adr_0015.md)); update the references to the old folders; then delete both folders | the build and `ctest` give the same results as before | none | Done |
| Continuous Integration | Linux and Windows jobs with clang, clang-tidy, the sanitizers on Linux, and the Windows artifact ([ADR 0015](../decisions/adr_0015.md), [DEP 0006](../dependencies/dep_0006.md)); a filter for single test functions in `check.hpp` ([ADR 0014](../decisions/adr_0014.md)) | all tests build and run on both jobs; `Formatting` passes | Migration | Open |
| Library Skeleton | Trace writer ([ADR 0007](../decisions/adr_0007.md)), results ([ADR 0008](../decisions/adr_0008.md)), Builder and Questions ([ADR 0005](../decisions/adr_0005.md), [ADR 0016](../decisions/adr_0016.md)), JSON and CSV parsers, Context with Persons and Relations, `FT_Free`, `FT_GetPersonCount`, `FT_GetPerson`, `FT_GetRelations`, `FT_IsDefaultString`; [FTR 0001](../features/ftr_0001.md), [FTR 0003](../features/ftr_0003.md), [FTR 0009](../features/ftr_0009.md) on the Library side | `builder_tests.cpp`, `parsers_tests.cpp`, `trace_tests.cpp` except `TheCallThatFailedShowsFailOnItsFirstLine`; from `queries_tests.cpp` the five tests of Details up to `AnEmptyListInThePersonFileIsNotMissing`; from `context_tests.cpp` `FreeingNoContextIsAnInvalidArgument` and `QueriesWithoutContextAreInvalidArguments`; from `resolver_tests.cpp` `RelationsOfAPersonContainItAsFirstOrSecond` | Continuous Integration | Open |
| Resolver | Parents, Children, Siblings, Partners, Families and Generations, conflicting Generations ([REQ 0045](../requirements/req_0045.md), [REQ 0047](../requirements/req_0047.md)), Relations to unknown Persons, and their queries: `FT_GetParents`, `FT_GetChildren`, `FT_GetSiblings`, `FT_GetPartners`, `FT_GetFamiliesOf`, `FT_GetFamily`, `FT_GetFamilyCount`, `FT_GetRelativeGeneration`, `FT_GetCommonParents`, `FT_GetCommonChildren`; [FTR 0004](../features/ftr_0004.md) on the Library side | the rest of `resolver_tests.cpp`; `TheCallThatFailedShowsFailOnItsFirstLine`; `AFailedCreationReleasesEverything` | Library Skeleton | Open |
| Search and Family Tree | `FT_Search` ([REQ 0016](../requirements/req_0016.md)) and `FT_GetFamilyTree` ([ADR 0009](../decisions/adr_0009.md), [REQ 0022](../requirements/req_0022.md)); [FTR 0002](../features/ftr_0002.md), [FTR 0004](../features/ftr_0004.md), [FTR 0009](../features/ftr_0009.md) on the Library side | the rest of `queries_tests.cpp`; `TwoContextsDoNotInfluenceEachOther`, `FreeingOneContextLeavesTheOtherUsable` | Resolver | Open |
| Media and Default Platform | Media paths and their check at creation ([ADR 0006](../decisions/adr_0006.md), [REQ 0011](../requirements/req_0011.md), [REQ 0023](../requirements/req_0023.md) to [REQ 0025](../requirements/req_0025.md)), `FT_GetPicturePaths`, `FT_GetRecordingPath`, the Default Platform on Windows and Linux ([REQ 0037](../requirements/req_0037.md)), `FT_Create`, the Media defaults and convenience functions; [FTR 0005](../features/ftr_0005.md), [FTR 0006](../features/ftr_0006.md), [FTR 0009](../features/ftr_0009.md) on the Library side | `media_tests.cpp`, `default_platform_tests.cpp`, `FreeingAContextReleasesEverythingItReturned`; afterwards all of `FamilyTreeTests` passes | Search and Family Tree | Open |
| Command Line Application | `applications/command_line/`: reads commands from a script or the standard input and prints the results ([ADR 0012](../decisions/adr_0012.md)); the commands every scenario of [FTR 0001](../features/ftr_0001.md) to [FTR 0004](../features/ftr_0004.md) and [FTR 0009](../features/ftr_0009.md) needs | none yet; the end-to-end tests of step 10 run it | Media and Default Platform | Open |
| Person File Document | `PersonFileDocument` with QJsonDocument ([REQ 0032](../requirements/req_0032.md), [REQ 0033](../requirements/req_0033.md), [REQ 0034](../requirements/req_0034.md), [REQ 0052](../requirements/req_0052.md)); [FTR 0007](../features/ftr_0007.md), [FTR 0008](../features/ftr_0008.md) without the form | `applications/data_entry/tests/` | Media and Default Platform | Open |
| Data Entry Form | the form, the file dialogs and the translations of the Data Entry Tool ([REQ 0031](../requirements/req_0031.md), [REQ 0034](../requirements/req_0034.md), [REQ 0004](../requirements/req_0004.md), [REQ 0046](../requirements/req_0046.md), [REQ 0052](../requirements/req_0052.md)); [FTR 0007](../features/ftr_0007.md), [FTR 0008](../features/ftr_0008.md) | none yet; Qt Test for the German labels ([ADR 0012](../decisions/adr_0012.md)) | Person File Document | Open |
| Browsing Basics | the Browsing Application: choosing the Data Folder, warnings and errors in German, search and Details ([REQ 0026](../requirements/req_0026.md) to [REQ 0028](../requirements/req_0028.md), [REQ 0004](../requirements/req_0004.md), [REQ 0046](../requirements/req_0046.md)); [FTR 0001](../features/ftr_0001.md), [FTR 0002](../features/ftr_0002.md), [FTR 0003](../features/ftr_0003.md) | none yet; Qt Test | Media and Default Platform | Open |
| Family Tree View | the Family Tree in a `QGraphicsScene` ([REQ 0029](../requirements/req_0029.md), [ADR 0010](../decisions/adr_0010.md)), with the layout from [SPK 0001](../spikes/spk_0001.md); [FTR 0004](../features/ftr_0004.md) | none yet; Qt Test | Browsing Basics, SPK 0001 concluded | Open |
| Stories and Pictures | the story player and the picture carousel ([REQ 0012](../requirements/req_0012.md), [REQ 0030](../requirements/req_0030.md), [REQ 0048](../requirements/req_0048.md)); [FTR 0005](../features/ftr_0005.md), [FTR 0006](../features/ftr_0006.md) | none yet; Qt Test | Family Tree View | Open |
<!-- TEMPLATE
| Work Package | Scope | Tests | Depends On | State |
| ---: | :--- | :--- | :--- | :--- |
| NAME | what it covers, with links to related documents where they exist | test files or folders | WORK PACKAGES | Open, In Progress, OR Done |
-->

## Notes per Work Package
<!-- What the implementer of each work package should know: where to start, which tests to make pass first, and what to watch out for. -->
### Migration
- After this package the user rewrites the git history, so the old code cannot be read anywhere afterwards, not even with `git show`. Whatever a later package needs from it has to be in the documents before the folders are deleted: go through the notes of every later package, check that what they take from the old code is written down there, and add what is missing. The notes already describe the question loop, the file and Media functions of the old platforms, and the commands of the old command line interface. Everything left behind has to be decided with the user in this package; there is no later.
- The example data, `original_implimentation/data/` (identical in `partial_rewrite/data/`), moves to `tests/data/example/`, the place for Data Folders used as test data ([conventions](../conventions.md#tests)). [SPK 0001](../spikes/spk_0001.md) and the end-to-end tests use it. Convert it on the way, with a small script or by hand, as the [handover](handover.md#pitfalls) describes:
    - every Person File gets the Format Version `1F10B769-08DE-442F-85BA-737B7F2B4803`, and its "Remarks" string becomes an array with one string per line;
    - `relations.csv` gets the line `Version,227E82BA-6603-426C-9C95-5BC86A10A642` in front of its header;
    - `1_Philippa_Perry/data.json` keeps its unknown key `death`, on purpose, as the case of [REQ 0005](../requirements/req_0005.md);
    - `1_Philippa_Perry/data.json` references `Sunday_plans.mp3`, but the file is `Sunday_Plans.mp3`. Linux treats them as two files, Windows as one, so the Recording is missing on Linux only. Ask the user whether to fix the reference; the vision expects every Recording of the example to be reachable.
    - `relations.csv` holds the row `3,4,Unrestricted,Marriage,…` twice. Leave it, and settle what a duplicate Relation means in the package [Resolver](#resolver).
- Do not move `relations_extra.csv`, `4_Constance_Houghton/data_extra.json` (not valid data, see the [handover](handover.md#testing-notes)) and `macro.txt` (a script for the old command line interface).
- `data/BEISPIEL_PERSON.json` is a German template for the data collector, in the old Format Version. Ask the user whether it is still handed out now that the Data Entry Tool exists; if so, convert it and keep it.
- `LICENSE` (MIT) moves to the root of the repository; the new tree has none, and the Windows artifact has to include the license texts ([ADR 0015](../decisions/adr_0015.md)).
- `.github/workflows/cmake-multi-platform.yml` and `doxygen.yml` move to `.github/workflows/`. Neither works as it is: the first configures with MSVC and without Qt, the second expects a `Doxyfile` that no longer exists. The package [Continuous Integration](#continuous-integration) adapts them; it runs right after this one, and the workflows only trigger on `main`.
- Left behind, and to be confirmed with the user before deleting, since nothing can be ported later: the Trace viewer `telemetry_viewer` (a development tool; the Trace of [ADR 0007](../decisions/adr_0007.md) is readable text, see the [handover](handover.md#other)), the theme `docs/doxygen/css` (doxygen-awesome-css, external code that would need a DEP record), `CMakeSettings.json` (Visual Studio), Dear ImGui in `external/` (replaced by Qt, [DEP 0001](../dependencies/dep_0001.md)), and the old records in `docs/`, whose content is in the new records. The one exception is TDR 0005 of the original, the remark field that does not wrap its lines, which is noted for the package [Data Entry Form](#data-entry-form).
- References to update once the folders are gone: the paths in the [handover](handover.md) (`original_implimentation/data/` becomes `tests/data/example/`; the reference to `AskNextQuestion` in `partial_rewrite/lib_family_tree/src/setup.c` becomes a link to the description in [Library Skeleton](#library-skeleton)), these notes, and the comments in `CheckFormat.py` and `cmake/ClangFormat.cmake` that name the old folders. The records only mention "the original implementation" as history, and stay as they are.

### Continuous Integration
- Start from the moved workflow. Use clang on both jobs ([DEP 0005](../dependencies/dep_0005.md)), install Qt 6 and Doxygen on the runners, drop the unused `-DFT_CICD`, and build the target `documentation` instead of the separate Doxygen workflow.
- Linux runs every test with AddressSanitizer and LeakSanitizer (the default of `FT_SANITIZERS`). On Windows the sanitizers are off, and the leak check of `check.hpp` only prints a message without failing the run, so leaks are only caught on Linux.
- The Windows artifact (`windeployqt`, the German Qt translations, the license texts) only has something to bundle once the Data Entry Tool has a form. Prepare the step, and finish it with the package [Data Entry Form](#data-entry-form); the [handover](handover.md#other) lists the plugins and translations to check.
- A filter for single test functions is a small addition to `check.hpp`, e.g. an environment variable with the name of the test that `RUN` compares against. [ADR 0014](../decisions/adr_0014.md) expects such features to be added by hand when needed. Ask the user before adding it.

### Library Skeleton
- The largest package, because every test of the Library goes through the whole chain: `DataFolder::Create` asks the Builder, which needs the parsers, which fill the Context, which only the queries can read. No component can be tested alone. If one session is not enough, make these tests pass first, as they need the least: `CancellingACreationReleasesTheBuilder`, `ContinueWithoutAnswerIsAnInvalidArgument`, `StartCreateWithoutDataFolderIsAnInvalidArgument`, `FreeingNoContextIsAnInvalidArgument`, `QueriesWithoutContextAreInvalidArguments`, then `UsesThePersonAnApplicationParsedItself` and `AnAnswerTellsEmptyDetailsFromMissingOnes`, which need the CSV parser but not the JSON parser. Then the JSON parser opens the rest.
- `FT_Context` and `FT_Builder` are opaque in the header; their structs go into an internal header in `library/src/`, shared by the source files. Static functions are named `Priv…`, other internal functions have no prefix (clang-tidy checks this).
- The first Question is always LIST_FOLDERS of the Data Folder, with exactly the path the Application passed: `ReadsTheDataFolderOnlyThroughQuestions` compares it with `family`, so add no slash. Every other path joins the Data Folder and the relative parts with `/` ([ADR 0016](../decisions/adr_0016.md)); do not double the slash when the Data Folder already ends with one.
- The question loop of the partial rewrite, `AskNextQuestion`, was a state machine with one state per Question type. On each call it stored the answer to the last Question, decided the next Question from the type of the last one, and kept its progress (the list of folders and the index of the current one) in the Builder: after LIST_FOLDERS it asked READ_FILE for the `data.json` of the first folder; after READ_FILE it asked the matching parsing Question for the text just read; after a parsed Person File it moved on to the next folder, and after the last folder it built the Context and freed the Builder. An answer with `useDefault` was first answered by the Default Platform, and then went through the same step. It asked for `relations.csv` first; the new order starts with the folders ([ADR 0016](../decisions/adr_0016.md)), and adds FILE_EXISTS for the Media.
- The Builder copies everything it keeps from an answer before `FT_Continue` returns: the test helper frees the strings of an answer right after the call. When the creation ends, successfully or not, the Builder frees itself, and the final result is `FT_OK_WITH_WARNINGS` if any warning came up during the whole creation.
- Missing Details point to the one default string of the Context, an empty text, which `FT_IsDefaultString` recognizes by its address. A text that is empty in the Person File points to a different empty text. An empty list still has `items[0]`: the default string if the list is missing, the other empty text if it is empty in the file, and the same for the placeholder of the Places of Residence ([ADR 0017](../decisions/adr_0017.md)). The partial rewrite cast the default string itself to `char**` for empty lists, so `items[0]` read the characters of the string as a pointer; do not copy that.
- In an answer to a parsing Question, a null text and a list with null `items` mean missing, an empty text and a list with `items` set and a count of 0 mean present but empty. `AnAnswerTellsEmptyDetailsFromMissingOnes` checks it.
- Rules of [ADR 0011](../decisions/adr_0011.md) that bite in the parsers: no recursion, so the JSON parser walks nested arrays and objects with an explicit stack of fixed depth; no function pointers, so `qsort` and `bsearch` are out, and the index of Persons by Person Id needs an own iterative sort and binary search; every loop has an upper bound, e.g. the length of the text; no function longer than 60 lines. clang-tidy checks recursion and length, function pointers are left to review.
- The JSON parser decodes every escape sequence, including `\uXXXX` and surrogate pairs ([handover](handover.md#pitfalls)), and checks every value for valid UTF-8, skipping only that value ([REQ 0051](../requirements/req_0051.md)). A wrong type skips the key ([REQ 0003](../requirements/req_0003.md)), an unknown key is reported and ignored ([REQ 0005](../requirements/req_0005.md)), a malformed file is skipped as a whole ([REQ 0013](../requirements/req_0013.md)). The parser must read "Images" and "Audio" already, or they are reported as unknown keys; resolving them belongs to [Media and Default Platform](#media-and-default-platform).
- The CSV parser reads quoted fields with commas and doubled quotation marks ([REQ 0007](../requirements/req_0007.md)), skips a row with fewer than three columns or invalid UTF-8, and reads an empty or unknown Relation Type as Unrestricted with a warning ([REQ 0008](../requirements/req_0008.md)). The second line is ignored whatever it holds (`IgnoresTheSecondLineOfTheRelationsFile`).
- A Relation that names a Person Id without a Person File is left out with a warning ([ADR 0017](../decisions/adr_0017.md)). `FT_GetRelations` returns a view, so the Relations of each Person are collected once at creation.
- The Resolver does not exist yet in this package: creation goes straight from the parsed data to the Context. Leave a clear place for it.
- Gap: two Person Files with the same Person Id. [REQ 0002](../requirements/req_0002.md) forbids it in the data, the vision assumes the Library reports it, and [OQ 0008](../open_questions.md) is still open. The index by Person Id cannot hold both. Clear it up with the user before writing the index, e.g. skip the second one with a warning, and record the answer with `/new-record`.
- The Trace: the first line of each public call is written with `[....]` and overwritten with its outcome when the call returns, also when the buffer has filled up in between. With a capacity of 0 nothing is written, and a null Trace is replaced by an empty one on entry, so internal functions never check for it. `CountsTheLinesThatDoNotFit` uses a buffer of 64 bytes.
- Once a stub file has no stub left, remove its `NOLINTBEGIN(readability-non-const-parameter)` block.

### Resolver
- Generations are the smallest numbers that satisfy [REQ 0045](../requirements/req_0045.md): a Child below each Parent, all Parents of a Child in the same Generation, everyone as high as possible. Merge the Parents of each Child into one node and compute the longest path from the top, iteratively, e.g. with Kahn's topological sort. A cycle in that graph is exactly a conflict of [REQ 0047](../requirements/req_0047.md); report the Relations involved, with their Person Ids, as errors, and free everything.
- A Child need not be exactly one Generation below a Parent. If P is the only recorded Parent of C, and C has a Child with Q, who is five Generations deep, then C moves down to Q's Generation while P stays at 0, as high as the rules allow. The documentation of `FT_GetRelativeGeneration` in `api.h` says "A Child is therefore 1 relative to its Parent", which is only true when nothing pushes the Child further down. Point it out to the user before relying on it; the Family Tree view will show a long line in such a case.
- Read the [handover](handover.md#pitfalls) on Generations: the original assigned them in a single pass from the top, recursively, which is wrong in both respects.
- A Family is a set of Parents together with all Children they have in common. Group the Children by their exact set of Parents: the half-siblings in `CommonParentsOfHalfSiblings` belong to two Families. A Child with one recorded Parent forms a Family with that Parent alone.
- Siblings and Partners are views, so compute them at creation; the Context allocates nothing afterwards ([ADR 0003](../decisions/adr_0003.md)). `FT_GetCommonParents` and `FT_GetCommonChildren` fill caller buffers instead.
- Gap: a Relation that appears twice, as in the example data, or a Strictly Lower and a Strictly Higher Relation that say the same. No requirement covers it. A Parent must not be counted twice either way; whether `FT_GetRelations` returns both rows, and whether it is worth a warning, is for the user to decide.

### Search and Family Tree
- Ask the user first whether the search ignores upper and lower case ([handover](handover.md#other), [OQ 0017](../open_questions.md)), and record the answer. Ignoring case only for ASCII is simple; for umlauts it needs a table, because the Library may not use external code.
- Every value of a list, and every name or date of a Place of Residence, counts as one given Detail; an empty or null text is not given; a missing Detail never matches ([REQ 0016](../requirements/req_0016.md)). With no given Detail and a minimum of 0, every Person matches. The search compares with the Context's texts, so it has to tell the default string apart by its address, not by its content.
- The Family Tree walks from Family to Family, never from Person to Person ([handover](handover.md#pitfalls), [ADR 0009](../decisions/adr_0009.md)). The Person around whom it is built is always part of it, also without any Family. The walk is a breadth-first search with a queue in a buffer of fixed size, e.g. the number of Families.
- A buffer smaller than the upper bound fails before anything is written, also if the actual result would fit (`FamilyTreeFailsWithATooSmallBuffer`, `SearchFailsWithATooSmallBuffer`). Nothing may be written to the outputs on an error, not even the count.
- Queries allocate nothing, and take the Context as `const`, so the walk cannot keep scratch memory in the Context either. Use the caller buffers as working memory: the Families buffer is the queue of the breadth-first search, walked level by level to count the Distance, and whether a Family or a Person was already collected is a search in the part of the buffer filled so far. That is quadratic, but bounded by the upper bounds, and fast enough for a few thousand Persons.

### Media and Default Platform
- The Builder resolves each Media path relative to the Person Folder, `<Data Folder>/<Person Folder>/<name>` with `/`, and asks FILE_EXISTS for each; a missing file is a warning naming the file, and is left out ([ADR 0006](../decisions/adr_0006.md)). `FT_GetRecordingPath` returns a null pointer when there is none.
- Default Platform on Linux: `opendir` and `readdir` for folders, `stat` for files and folder entries (`d_type` can be unknown on some file systems), `fopen` with `"rb"`, and the size from `fseek` and `ftell`. The old Linux platform of the partial rewrite did the same, but leaked on every error path and ignored the errors.
- Default Platform on Windows: paths are UTF-8, so convert them to UTF-16 with `MultiByteToWideChar` and use `FindFirstFileW`, `GetFileAttributesW` and `_wfopen` in binary mode. The old code used the `…A` functions, which break on umlauts (`ReadsFolderNamesWithUmlauts`), and read files in text mode, which changes line endings and sizes. The code for each operating system is chosen with `#ifdef _WIN32`; [ADR 0011](../decisions/adr_0011.md) allows the preprocessor only for includes, guards and the export macro, so check with the user how this platform switch fits, e.g. as one file per operating system chosen by the build instead.
- A Data Folder that cannot be listed is `FT_ERROR_PLATFORM` (`AMissingDataFolderIsAPlatformError`); an operating system without Default Platform returns `FT_ERROR_NOT_SUPPORTED`.
- The Media defaults: `ShellExecuteW(NULL, L"open", path, NULL, NULL, SW_SHOWNORMAL)` on Windows, with the path converted to backslashes and UTF-16; the original did the same with `ShellExecuteA`. On Linux e.g. `xdg-open` in a child process. Whether a program opens cannot be tested; only the arguments are (`TheDefaultsNeedAPath`). Run the tests without starting programs: no test passes a valid path to the defaults.
- The Default Platform tests write to the temporary folder of the operating system, into `family_tree_default_platform_test`.

### Command Line Application
- No document says which commands it has. Write them down in an ADR with `/new-record` first: one command for each Library function a scenario of [FTR 0001](../features/ftr_0001.md) to [FTR 0004](../features/ftr_0004.md) and [FTR 0009](../features/ftr_0009.md) uses, with a stable output format that the end-to-end tests compare. The original had `help`, `exit`, `add` (load a Person File or a Relations File), `print-person`, `play-audio`, `show-images`, `search`, `show-relations` and `print-tree`, each with a one-letter short form, and could run them from a script file.
- Opening a Data Folder through Questions answered by the Application is the way to test [FTR 0009](../features/ftr_0009.md), "Defaults for the rest", end to end; give the Command Line Application an option for it.
- It is a development tool ([ADR 0012](../decisions/adr_0012.md)): plain C++, no Qt, and English is fine.

### Person File Document
- The tests read the saved files back with `FT_Create`, which is why this package waits for the Library. `ShowsAnEmptyDetailOfAReopenedFileAsKnownNotToExist`, the refusing tests and the round trips through `Open` and `Save` do not need the Library, and can be worked on earlier.
- Keep the unknown keys of an opened file, and write them back unchanged ([REQ 0033](../requirements/req_0033.md), `ReopeningAndSavingKeepsUnknownKeys`). `QJsonObject` sorts keys alphabetically when it writes; that is allowed, as no requirement fixes the order.
- `std::nullopt` leaves a key out, an empty text or list writes it empty ([REQ 0052](../requirements/req_0052.md)). Remarks are stored as lines; `SplitIntoLines` and `JoinLines` convert between the text of the form and the lines.
- `DataEntryDocument` links Qt Core privately, so the interface keeps to standard types only ([architecture](../architecture.md#data-entry-tool)). The tests write into `family_tree_data_entry_test` in the temporary folder.
- The Format Version is hard coded in the Data Entry Tool, and has to change together with the Library ([ADR 0016](../decisions/adr_0016.md)).

### Data Entry Form
- Every text in German, kept apart from the code in Qt Linguist files ([REQ 0004](../requirements/req_0004.md), [REQ 0046](../requirements/req_0046.md)); `applications/translations/` is the place shared with the Browsing Application ([conventions](../conventions.md#folder-structure)).
- Each Detail needs a way to mark it as known not to exist ([REQ 0052](../requirements/req_0052.md)), next to leaving it empty.
- The remark field of the old tool did not wrap long lines (TDR 0005 of the original implementation): use a multi-line editor that wraps.
- Finish the Windows artifact of [Continuous Integration](#continuous-integration) here, and check it on a computer without Qt.
- Warnings and errors name the file concerned, in words a data collector understands ([conventions](../conventions.md#error-handling)).

### Browsing Basics
- Ask the user first how a Detail known not to exist is shown, e.g. "had no title of nobility", as opposed to an unknown one ([handover](handover.md#other)). `FT_IsDefaultString` on the text, or on `items[0]` of an empty list, tells the three cases apart.
- The result of the creation and the Trace become messages in German that name the file concerned. The Library's warnings are English text; the Application shows its own words, e.g. based on the result and a count of warnings.
- Paths from the Library contain `/` also on Windows ([ADR 0016](../decisions/adr_0016.md)); convert them with `QDir::toNativeSeparators` before showing them.
- Allocate the caller buffers once after the creation, with `FT_GetPersonCount` and `FT_GetFamilyCount`.

### Family Tree View
- Blocked until [SPK 0001](../spikes/spk_0001.md) is concluded; it is In Progress with no findings yet, and its records decide the layout within the rows.
- The rows come from the relative Generations of `FT_GetFamilyTree`; a Generation gap larger than one is possible, see the [Resolver](#resolver).

### Stories and Pictures
- Ask the user first which controls the story player gets beyond pausing and continuing, whether a Recording keeps playing while the user moves to another Person, and what the carousel does after the last Picture ([handover](handover.md#other)).
- The Recording is started from the Person in the Family Tree ([REQ 0012](../requirements/req_0012.md)), which is why this package comes after the [Family Tree View](#family-tree-view).
- The multimedia backend and the image format plugins of Qt have to be in the Windows artifact.
<!-- TEMPLATE
### WORK PACKAGE
- NOTE
-->

## General Notes
<!-- What applies to all work packages, e.g. shared code, test helpers, or conventions that came up while implementing the tests. -->
- After the package [Migration](#migration), the old code is gone, also from the git history. What the packages need from it is written into their notes; do not look for it elsewhere. Before the Migration, treat it as a model, not a source to copy: it uses function pointers, recursion, ANSI file functions and allocations after creation, which the new rules forbid ([ADR 0011](../decisions/adr_0011.md)).
- The Library tests call the public API only, and get their data from the in-memory `DataFolder` of `test_support.hpp`, so the inside of the Library can be arranged freely. `DataFolder` answers LIST_FOLDERS, FILE_EXISTS and READ_FILE from its files, and leaves both parsing Questions to the Library unless a test calls `AnswerPersonParsing`.
- The whole test suite runs at the end of every package, to catch side effects on finished packages. The Library tests are one executable, so a failing test of a later package shows up in every run; compare against the list of tests in the [Work Packages](#work-packages) table, not against "all green".
- If a test looks wrong, do not change it to make the code pass: point it out, and fix the documentation first if that is where the error is.
- Each package replaces the stubs of its functions only. Functions of later packages keep returning `FT_ERROR_NOT_SUPPORTED`.
- Questions that are left to the user are listed in the notes of the package they belong to. Record each answer where it belongs, usually with `/new-record`, before implementing it.
<!-- TEMPLATE
- NOTE
-->

-----
## Work Log
<!-- One entry per session, newest at the bottom: what was implemented, and the quirks found along the way. -->
<!-- TEMPLATE
### YYYY-MM-DD WORK PACKAGE
- **Implemented:** what was done, and which tests pass now
- **Quirks:** surprises, workarounds, and things left open
-->

### 2026-11-15 Migration
- **Implemented:** The example data is in `tests/data/example/`: 18 Person Files in the Format Version `1F10B769-08DE-442F-85BA-737B7F2B4803`, the remark of `1_Philippa_Perry` as a list of lines, and `relations.csv` with the line `Version,227E82BA-6603-426C-9C95-5BC86A10A642`. `Sunday_Plans.mp3` (royalty free) and `Trohnsaal.png` (the user's own photo) were copied, and the reference to the Recording was corrected from `Sunday_plans.mp3` to `Sunday_Plans.mp3`. English templates replace the German `BEISPIEL_PERSON.json`: `templates/person/data.json` and `templates/relations.csv`, added to the folder structure in the conventions. `LICENSE` and both workflows were moved unchanged to the root and to `.github/workflows/`. The handover, the entry points of these notes, `CheckFormat.py` and `cmake/ClangFormat.cmake` no longer name the old folders. Left behind by decision of the user: the Trace viewer, the Doxygen theme, `CMakeSettings.json`, Dear ImGui, the old records, `relations_extra.csv`, `data_extra.json` and `macro.txt`. Both reference folders are deleted. The build with clang-tidy has no warnings, and `ctest` gives the same results as before: `Formatting` passes, `FamilyTreeTests` fails on the stubs (110 tests), `DataEntryTests` passes 2 of 12; `documentation` passes.
- **Quirks:** The user rewrites the git history after this package, so the old code is gone for good; what later packages need from it is in their notes. The moved workflows do not work yet (MSVC, no Qt, a missing `Doxyfile`); the package Continuous Integration adapts them, and they only trigger on `main`. The Person File template has the Id 0, because JSON needs an integer there; the old template had a placeholder text, which was not valid JSON. The duplicate row `3,4,Unrestricted,Marriage` in the example `relations.csv` was kept on purpose for the package Resolver. At the user's request, the example data was then rewritten to cover more edge cases, keeping the 18 Persons, their Person Ids and all Parents and Children: every Person File covers other cases (formatting, escape sequences, other scripts, empty and missing Details, wrong types, unknown nested keys, missing Media, a Person Folder with an umlaut and a space, now `13_Käthe Ball`), and so does `relations.csv`; the handover lists them in a table. Two small Pictures were added for `8_Megan_Christensen`. The example now loads with several intended warnings.
