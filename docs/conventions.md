---
Author: Andreas Edmeier
Created at: 2026-11-15
Last reviewed: 2026-11-15
---
# Family Tree Coding Conventions

## Tooling
<!-- The formatters and linters in use, and where their configuration lives. Rules enforced by these tools are not repeated in this document. -->
| Tool | Purpose | Configuration |
| ---: | :--- | :--- |
| [clang-format](dependencies/dep_0003.md) | formatting of C and C++ | `.clang-format`, taken over from the original implementation |
| [clang-tidy](dependencies/dep_0004.md) | static analysis of the Library, with checks for C and the Power of Ten rules | `library/.clang-tidy` |
| [clang-tidy](dependencies/dep_0004.md) | static analysis of the Applications and tests, with checks for C++ | `.clang-tidy`, taken over from the original implementation |
| [clang](dependencies/dep_0005.md) | the compiler for the Library, the Applications and the tests, on Windows and Linux; the Library is compiled with `-std=c99 -pedantic-errors -Wall -Wextra -Werror` | `library/CMakeLists.txt` |
| AddressSanitizer, LeakSanitizer | memory errors and leaks while the tests run on Linux | `CMakeLists.txt` |

- clang-tidy runs on the C files too, through `CMAKE_C_CLANG_TIDY`; the original implementation only ran it on C++. CMake passes each language its configuration file explicitly (`cmake/ClangTidy.cmake`), because the tests of the Library are C++ but live below `library/.clang-tidy`.
- Test targets turn off the checks for magic numbers, C arrays and unchecked access (`ft_clang_tidy_for_tests` in `cmake/ClangTidy.cmake`): Person Ids are test data, the tests call the C API with caller buffers, and `CHECK` returns before an unchecked access.
- clang-tidy runs while building; `-DFT_CLANG_TIDY=OFF` turns it off. The build target `format` formats every file in `library/`, `applications/` and `tests/`, and `check-format` and the test `Formatting` fail if one is not formatted.
- `CheckFormat.py` checks the same files without CMake, e.g. from a pre-commit hook with `./CheckFormat.py --files staged --fix`; it formats the files it finds and fails, so they can be added again.
- C++ code that includes the public headers of the Library only gets diagnostics for its own files, through `HeaderFilterRegex`, so the headers need no `NOLINT` blocks.

## Folder Structure
<!-- What goes where in the source and test folders. -->
```
library/
    include/family_tree/    public headers, the only part Applications include
    src/                    implementation of the Library
    tests/                  unit tests of the Library
applications/
    browsing/               Browsing Application
    data_entry/             Data Entry Tool
    command_line/           Command Line Application
    translations/           Qt Linguist files shared by the Applications
cmake/                      CMake modules: warnings, clang-tidy, clang-format
tests/
    framework/              the own test framework, check.hpp
    end_to_end/             end-to-end tests from the scenarios of the features
    data/                   Data Folders used as test data
spikes/
    spk_<number>/           spike code, never part of the product
```

## Naming
<!-- Naming rules beyond capitalization, e.g. suffixes for certain kinds of classes, verbs for functions, or names for recurring concepts. -->
| Element | Convention | Example |
| ---: | :--- | :--- |
| public function of the Library | `FT_` and CamelCase, starting with a verb | `FT_GetSiblings` |
| public type of the Library | `FT_` and CamelCase | `FT_Context`, `FT_PersonId` |
| public enum constant | `FT_` and upper case | `FT_OK_WITH_WARNINGS` |
| macro | upper case, `FT_` in public headers | `FT_API` |
| internal function | CamelCase, no prefix | `ResolveGenerations` |
| function with internal linkage | `Priv` and CamelCase | `PrivParseResidence` |
| parameter | `a` and CamelCase | `aContext` |
| output parameter | `aOut` and CamelCase | `aOutCount` |
| local variable, struct member | camelBack | `personCount` |
| global constant | `the` and CamelCase | `theCompatibleVersion` |
| C++ member variable | `my` and CamelCase | `myPlayer` |
| words of the domain | the terms of the [glossary](glossary.md) | `Partner`, not `Spouse` |

## Error Handling
<!-- How errors are signaled, where they are caught, and what is reported to the user. -->
- Every public function of the Library returns `FT_Result`, and returns its results through output parameters, see [ADR 0008](decisions/adr_0008.md).
- Every public function checks its parameters; internal functions state their preconditions with `assert`, see [ADR 0011](decisions/adr_0011.md).
- Every return value is checked.
- A problem in the data is a warning: the Library works around it, reports it to the Trace, and returns `FT_OK_WITH_WARNINGS`. A call that cannot do what was asked returns an error.
- The Applications show warnings and errors to the user in German, in words a data collector understands, and name the file concerned.

## Logging
<!-- What gets logged, and at which level. -->
The Library writes its reports as lines into the Trace buffer of the Application, see [ADR 0007](decisions/adr_0007.md). Every public function, and every internal function that does a task worth skipping when it worked, e.g. reading one Person File, opens a Subtrace and closes it with its outcome. Decisions the code takes, e.g. which branch of a condition, are reported as steps.

| Level | Used for |
| ---: | :--- |
| STEP | what the Library does, to follow its flow, e.g. reading a Person File or resolving the Generations |
| WARNING | a problem in the data that the Library worked around, e.g. a skipped Person File, an unknown key, an unknown Relation Type, or a missing Media file |
| ERROR | a failure that keeps a call from doing what was asked, e.g. conflicting Generations or an unknown Person Id |

## Comments
<!-- The comment style, e.g. Doxygen, and what must be documented. -->
- Every public function and type of the Library is documented in its header file in Doxygen format with `///`, in English, as the original implementation did.
- The documentation of a function says whether it returns a view into the Context or fills a caller buffer, and how large the buffer has to be.
- Comments in the code explain why, not what.

## Tests
<!-- How test files and test functions are named, where test data lives, and how tests reference the scenarios they cover. -->
- Tests are written in C++ with the project's own test framework `tests/framework/check.hpp`, see [ADR 0014](decisions/adr_0014.md). Qt Test is only used for the parts of the graphical Applications that need simulated input.
- Unit tests of the Library live in `library/tests/`, one file per module. Every module has a suite function `bool <Module>Suite()` that runs its tests with `RUN(...)`; every test is a function `bool <Behavior>()` named after the behavior it checks, e.g. `MatchesAnyOfSeveralFirstNames`.
- Every test references the requirement it checks by the path of the record, e.g. `docs/requirements/req_0016.md`, in a comment above the test.
- End-to-end tests live in `tests/end_to_end/`, run the Command Line Application, and reference the scenario they cover by path and anchor, e.g. `docs/features/ftr_0004.md#the-closest-family`.
- Test data lives in `tests/data/`, one Data Folder per situation, named after what it contains, e.g. `conflicting_generations/`.
- The tests of the Library run with AddressSanitizer and LeakSanitizer on Linux, and fail on any leak.
