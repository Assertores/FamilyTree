---
name: implement
description: Starts the implementation of a work package from the implementer's notes. Reads everything related, clears up contradictions and uncertainties with the user one question at a time, then implements until the package's tests pass. Use when the user wants to start or continue implementing a work package.
argument-hint: "[work package, optional]"
---

# Implement

## 1. Pick the work package

1. Read `docs/workspace/implementers_notes.md`.
2. Take the work package from the arguments. Otherwise suggest the first package in the Work Packages table whose state is `Open` and whose dependencies are `Done`, and ask the user to confirm.
3. Set its state to `In Progress`, and the `State` of every FTR in its scope that is not `Rejected` to `In Progress`.

## 2. Read

- The package's row in the Work Packages table, its notes, and the General Notes.
- The Work Log entries of the packages it depends on, and earlier entries of this package.
- Every document in the package's scope: requirements, ADRs, features, spikes, architecture, glossary, conventions.
- The package's tests, and the source code it builds on.
- Skipped tests, in this package and in packages that are `Done`, whose open question is listed under Answered in `docs/open_questions.md`: their question was answered, so they are part of this session's work. Ask the user whether to bring them back.

## 3. Clear up uncertainties

Before writing any code, settle two things with the user.

**The structure.** The user decides the ideas behind the structure of the code: which patterns, algorithms and responsibilities it is built from, e.g. "a mediator between the handlers", "a chain of responsibility", "a depth-first search over the tree". Not the code itself. For every part of the package whose structure is not already given by the architecture or an ADR, describe the problem it has to solve, suggest one or two approaches with their trade-offs, and ask the user how it should be structured. If the user's answer is a significant structural decision, suggest recording it as an ADR with `/new-record`, with the structure in its Design section.

**The open points.** Look for:

- tests that contradict each other, or contradict a requirement or ADR;
- tests, requirements or ADRs that are unclear or allow several readings;
- gaps in the requirements: behavior the package needs that no requirement covers;
- anything else you are unsure about.

Ask about the structure first, then about the open points, **one question at a time**: one question per message, with the context needed to answer it and, where possible, a suggested answer. Wait for the answer before asking the next question. Continue until nothing important is left open.

If an answer changes the documentation, e.g. a new design decision, a requirement that has to change, or a test that is wrong, point it out and suggest the change: a new ADR or a replacement record with `/new-record`, or a correction of the test. Make documentation changes only once the user agrees. Tests are only changed after the documentation they come from is correct.

A gap in the requirements is closed with a new REQ before any code is written for it:

1. Create the REQ with `/new-record`, and refine it with the user until they say it is settled. Do not continue before that.
2. Check it against the existing requirements with `/check-requirements <ID>`, and settle any finding with the user.
3. Write its tests with `/requirements-to-tests <ID>`. If that invents new interfaces, consolidate them with the user in this session (an ADR or the architecture), then delete `docs/workspace/interface_draft.md`.
4. Add the REQ and its tests to the package's row in the Work Packages table.

Some questions cannot be answered yet. If the user decides to postpone one, the tests that depend on it are deferred:

- Add the question under Open in `docs/open_questions.md` with the next free ID (the highest `OQ` number in the document, Open and Answered together, plus one), unless it is already there.
- Mark each affected test as skipped with the test framework's skip mechanism, with a reason that names the open question by its ID, e.g. `OQ 0003`. Never delete or comment out a test.
- Only defer tests the user agreed to defer.

## 4. Plan

Summarize in a few lines how the package will be implemented: the structure agreed in step 3, the modules and functions, the order, which tests should pass after each step, and which tests are deferred. Ask the user to confirm the plan.

## 5. Implement

- Follow `docs/conventions.md` and the decisions from step 3.
- Work in small steps and run the package's tests after each one.
- Before finishing, run the whole test suite. If tests of other packages fail, point them out: either this package broke them, or a new requirement changed behavior they rely on. Either way, decide with the user how to resolve it.
- Never change a test to make the code pass. If a test turns out to be wrong after all, stop and go back to step 3.
- If a new uncertainty comes up, ask the user before guessing.

## 6. Finish

When the package's tests pass, apart from the deferred ones, or the session has to end, run `/log-session`.

Then update the `State` of every FTR in the package's scope: `Implemented` if every work package whose scope contains it is `Done`, otherwise `In Progress`.
