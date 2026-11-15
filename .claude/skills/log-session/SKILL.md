---
name: log-session
description: Writes the work log entry for the current implementation session into the implementer's notes and updates the state of the work package. Use at the end of an implementation session, or when the user asks to log the session or the work done.
argument-hint: "[work package, optional]"
---

# Log Session

## 1. Gather

1. Read `docs/workspace/implementers_notes.md`.
2. Find the work package of this session: from the arguments, from the conversation, or from the changed files. If it stays unclear, ask.
3. Look at what changed in this session: the conversation, and `git status` and `git diff` (including staged changes).
4. Run the tests of the work package, as listed in its row of the Work Packages table, and note which pass.

## 2. Write the entry

Append one entry at the end of the Work Log, in the form of the template there:

```
### YYYY-MM-DD WORK PACKAGE
- **Implemented:** what was done, and which tests pass now
- **Quirks:** surprises, workarounds, and things left open
```

- Use today's date.
- **Implemented:** concrete, e.g. which modules or functions were written, and which tests pass now, which still fail, and which are deferred.
- **Quirks:** only what the next session needs to know, e.g. a test that is flaky, a workaround and why it was needed, a part of the documentation that turned out to be wrong. Write "none" if there are none.

## 3. Update the state

In the Work Packages table, set the package's state:

- `Done` if all of its tests pass, apart from tests the user deliberately deferred.
- `In Progress` otherwise.

List deferred tests under **Quirks**, each with the ID of the open question it waits for, e.g. `OQ 0003`.

## 4. Point out follow-ups

If the session found something that belongs in the documentation (a wrong requirement, a missing scenario, a design change, new technical debt), list it for the user and suggest the matching record, e.g. a new TDR with `/new-record`. Do not create it without asking.

Also check whether the session changed something that `docs/architecture.md`, `docs/conventions.md` or `docs/glossary.md` describes, e.g. a new component, a new naming rule, or a new term. If so, suggest the edit, and make it only once the user agrees.
