---
name: foreman
description: Reads the whole documentation and the tests, then writes the implementer's notes (getting started, work packages, notes per package) that prepare the implementation of the source code. Use when the unit tests are written and consolidated, and the user wants to prepare or plan the implementation phase.
---

# Foreman

Prepare the implementation: everyone who later implements a part of the source code reads `docs/workspace/implementers_notes.md` first. Write it so that a session that knows nothing about the project can start working on one work package right away.

## 1. Read everything

- All of `docs/`: vision, architecture, conventions, glossary, open questions, and all records. Skip the templates and the `0000` examples.
- `docs/workspace/handover.md`: implementation order, pitfalls and testing notes from the documentation phase.
- All tests, and the source code if any exists.
- `docs/workspace/interface_draft.md`, if it still exists. If it does, tell the user that the interfaces were not consolidated yet and ask whether to continue.

## 2. Verify

- Build the project and run the tests. Note the exact commands that work.
- Run a single test, and note the command.
- The tests are expected to fail because the source code is missing. Note any test that fails for a different reason (setup errors, missing test data, broken test code) as a finding.

## 3. Write the implementer's notes

Fill in the sections of `docs/workspace/implementers_notes.md`. Keep the hint comments. Do not touch the Work Log.

- **Getting Started:** the verified commands from step 2, and where the entry points of the source code and the tests are.
- **Work Packages:** split the implementation into parts that one session can complete and that can be done as independently as possible. Usually one per component, or per feature for large components. For each: its scope (with links to the related documents), the tests that must pass when it is done, what it depends on, and the state `Open`. Set the `State` of every FTR in a package's scope that is `Proposed` to `Planned`. Order the table so that packages come after the ones they depend on, following the implementation order from the handover.
- **Notes per Work Package:** for each package, where to start, which tests to make pass first, and what to watch out for: pitfalls from the handover, unclear parts of the documentation, and interfaces it shares with other packages.
- **General Notes:** shared code, test helpers, and what applies to all packages.

## 4. Report

Summarize the work packages in a few lines, and list the findings from step 2 and any contradictions or gaps found in the documentation while reading.
