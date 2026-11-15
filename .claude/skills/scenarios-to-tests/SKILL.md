---
name: scenarios-to-tests
description: Writes end-to-end tests from the Given/When/Then scenarios of the feature records (FTR), and reports which scenarios have no test yet. Use when the user asks to write or generate end-to-end tests from features or scenarios, or asks about scenario test coverage.
argument-hint: "[FTR IDs or a README topic, and 'coverage' for the report only]"
---

# Scenarios to Tests

## 1. Prepare

1. Read `docs/conventions.md`, especially the Tooling and Tests sections: test framework, file naming, folder, test data. If the test framework or folder is not defined there, ask the user and suggest adding it to the conventions.
2. Read `docs/architecture.md`, especially the Interfaces section: end-to-end tests only use the system from the outside, through these interfaces (CLI, API, UI, files, ...).
3. Read `docs/workspace/handover.md` for testing notes.
4. Read the existing tests, to follow their structure and reuse their helpers.

## 2. Find the scenarios

Read every `docs/features/ftr_*.md` except `ftr_template.md` and the example `ftr_0000.md`, or only the FTRs named in the arguments, or listed under the topic of `docs/features/README.md` they name. Skip features in state `Rejected`, and features whose `Replaced by` links to a different FTR.

A scenario is covered if a test references it by its link anchor, e.g. `ftr_0003.md#wrong-password` (the heading in GitHub slug form: lower case, spaces become `-`, punctuation removed). Search the test folder for these references.

If the argument is `coverage`, skip to step 5.

## 3. Write the tests

For every scenario without a test, write one end-to-end test:

- **Given** becomes the setup: test data, starting state, logged-in user, ...
- **When** becomes the action, performed through the system's external interface only.
- **Then** becomes the assertions. Assert exactly what the step says the user can observe, nothing more.
- The test references its scenario in a comment or its name, as in step 2, e.g. `// covers docs/features/ftr_0003.md#wrong-password`.
- Do not stub or fake any part of the system under test; only external systems may be mocked.

Then run the new tests. A failing test means either the implementation or the scenario is wrong: do not change the test or the code to make it pass, list it in the report instead.

If a scenario cannot be tested from the outside as written (e.g. a step that is not observable), do not write a test for it; list it in the report instead.

## 4. Update the feature states

Run the tests of every checked FTR that is `Implemented` or `Done`, and set its `State`:

- `Done` if every scenario has a test and all of them pass.
- `Implemented` otherwise.

Leave FTRs in any other state unchanged.

## 5. Report

- How many tests were written, and in which files. No list of individual tests.
- The failing tests, each with the scenario, what was expected, what happened, and whether the implementation or the scenario looks wrong.
- The scenarios still without a test, with the reason.
- Scenario steps that turned out to be ambiguous or not observable, as candidates to improve in the FTR.
- Testing details worth keeping, as a suggestion for the Testing Notes in `docs/workspace/handover.md`.
