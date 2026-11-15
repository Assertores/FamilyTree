---
name: requirements-to-tests
description: Writes unit tests from the Requirement Statements of the REQs, inventing the modules and interfaces the tests need, and collects all invented interfaces in a draft document for later consolidation. Use when the user asks to write or generate unit tests from requirements.
argument-hint: "[REQ IDs or a README topic to focus on, optional]"
---

# Requirements to Tests

Write unit tests that check the requirements, before the source code exists. Writing the tests forces decisions about modules and interfaces; record every one of them in `docs/workspace/interface_draft.md` so they can be consolidated afterwards.

## 1. Prepare

1. Read `docs/conventions.md` (test framework, naming, folder structure), `docs/architecture.md` (components and interfaces), the ADRs in `docs/decisions/`, and `docs/glossary.md`. If the test framework or test folder is not defined, ask the user.
2. Read the existing tests and source code, if any.
3. Read the concluded spikes in `docs/spikes/` that relate to the requirements: their findings, and the inputs and outputs in `spikes/`, can serve as test data. Copy the data into the test data folder instead of reading it from `spikes/`, so that spike code can be removed without breaking tests. A test built on them references the spike, e.g. `// data from SPK 0004`.
4. If `docs/workspace/interface_draft.md` exists, read it: interfaces invented in earlier sessions are reused, not invented again.

## 2. Collect the requirements

Read every `docs/requirements/req_*.md` except `req_template.md` and the example `req_0000.md`, or only the REQs named in the arguments, or listed under the topic of `docs/requirements/README.md` they name. Skip REQs whose `Replaced by` links to a different REQ.

Some requirements cannot be checked by a unit test (e.g. requirements about the process, the documentation, or the behavior of the whole system). Do not force them; list them in the report.

## 3. Write the tests

For every testable requirement:

1. **Decide where it belongs:** the component from the architecture that is responsible for it. If none fits, invent one and note that.
2. **Decide the interface:** the module, class or function the test calls, with its parameters, return value and errors. In this order of preference: an interface from the architecture or an ADR, one already in the draft document, a new one.
3. **Write the tests:** the normal case and the boundaries the requirement implies (e.g. exactly at a limit, just over it). Each test references its requirement in a comment or its name, e.g. `// checks REQ 0012`.
4. **Record the interface** in the draft document (step 4), including which test files use it.

The source code does not exist yet, so the tests are expected to fail or not compile. Do not write implementations; at most, write the empty declarations the language needs to compile the tests, and record them as invented.

## 4. The draft document

Write `docs/workspace/interface_draft.md` in this form, extending it if it already exists:

```
# Interface Draft
Modules and interfaces invented while writing unit tests from the requirements. Temporary: delete this document once the interfaces are consolidated into the architecture, ADRs and conventions.

## Modules
| Module | Component | Responsibility | Origin |
| ---: | :--- | :--- | :--- |
| NAME | component from the architecture, or NEW | what it does | Architecture, ADR 0003, OR invented |

## Interfaces
### MODULE
| Interface | Signature | Errors | Used by | Requirements |
| ---: | :--- | :--- | :--- | :--- |
| NAME | parameters and return value | what can fail and how it is signaled | test files | REQ IDs |

## Open Points
- overlaps between invented interfaces, conflicts with the architecture, and decisions taken without enough information
```

Mark everything you invented so it is easy to tell apart from what the documentation already defines.

## 5. Report

- How many tests were written, and in which files. No list of individual tests.
- The requirements that cannot be unit tested, with the reason.
- The new modules and interfaces, and the open points from the draft document.
- A reminder that `docs/workspace/interface_draft.md` needs a consolidation pass: move what is kept into the architecture, ADRs and conventions, adjust the tests, then delete the draft.
