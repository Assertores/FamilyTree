# Growing the Project
How to split the project into subsystems once it outgrows a single documentation tree, e.g. because several teams work on it in parallel, or because the skills that read the whole documentation no longer fit into one session. Not tried on a real project yet: adjust this guide with what the first split teaches.

## When to Split
- Split only when parts of the system have their own pace or their own people. Before that, group records by README topics, and pass a topic to the skills that accept one.
- Split along stable interfaces. A subsystem can be a container (a separately runnable part) or a large component inside one.

## Structure
The top level is the template itself, applied once more: its components are the subsystems. Each subsystem is a full copy of the template in its own folder, named with the subsystem's full name, in lower case with hyphens, e.g. `input/` or `scene-graph/`.

```
CLAUDE.md          rules for the whole repository
docs/              the top level: the whole system and how the subsystems work together
tests/             end-to-end tests of features that cross subsystems
input/             one folder per subsystem
    CLAUDE.md      what the subsystem is, and a link to the top level
    docs/          the subsystem's own vision, architecture, records and workspace
    ...            the subsystem's source and tests
rendering/
    ...
```

## What Goes Where
| Content | Top Level | Subsystem |
| ---: | :--- | :--- |
| Vision | the whole system | what the subsystem contributes to it |
| Architecture | the subsystems, and the interfaces between them | the subsystem's own components |
| Glossary | terms shared across subsystems | terms used only inside the subsystem |
| REQ | requirements that cut across subsystems, e.g. licensing, security, budgets | requirements of the subsystem, derived from the top level where needed |
| ADR | interfaces between subsystems, and decisions that affect several | decisions inside the subsystem |
| FTR | user stories that cross subsystems | user stories inside one subsystem |
| DEP | dependencies used by several subsystems | dependencies used only by the subsystem |
| TDR | debt that spans several subsystems | debt inside the subsystem |
| SPK | spikes whose question affects several subsystems | spikes inside the subsystem |
| Incident Procedure | the one procedure for the whole system | nothing; it follows the top level |
| Object Pages | none; containers belong to subsystems | one per container of the subsystem |
| INC | incidents that span several subsystems | incidents inside the subsystem |
| Open Questions | questions about the whole system or several subsystems | questions inside the subsystem |
| Tests | end-to-end tests of features that cross subsystems | unit tests, and end-to-end tests of its own features |

## Record IDs
- Records keep their plain IDs and file names, e.g. `REQ 0004` in `req_0004.md`. Numbering is per folder.
- In documents, every reference to a record is a relative link, across levels and subsystems too. The link text is the plain ID for a record of the same level, and the subsystem folder and ID for a record in a subsystem, e.g. `[input/REQ 0004](../../../input/docs/requirements/req_0004.md)` from a top-level REQ. A record of the top level, linked from a subsystem, has a leading slash, e.g. `[/REQ 0002](../../../docs/requirements/req_0002.md)`.
- In code and test comments, a reference is the file's path from the repository root, e.g. `input/docs/requirements/req_0004.md`, or `docs/features/ftr_0003.md#wrong-password` for the top level.
- Opened at the repository root as an Obsidian vault, the relative links form one knowledge graph across all levels.
- No record is ever renamed for a split.

## Features Across Subsystems
- The FTR lives at the top level, with its user story and scenarios. Its end-to-end tests run against the whole system.
- Each affected subsystem gets REQs for its part of the job. Their Context or Justification links up to the top-level FTR or REQ; the Requirement Statement stays free of links as always.
- Budgets are split the same way: a top-level REQ for the total latency becomes subsystem REQs whose limits together stay within it. Checking that they add up is a check across two levels.
- The work becomes one work package per affected subsystem. The FTR is `Implemented` once all of them are `Done`.

## Interfaces Between Subsystems
- Every interface between subsystems is owned by the top level: an ADR, plus a link to its detailed specification.
- Both sides test against it. A change to an interface is a top-level replacement record, and every subsystem that uses it adapts.

## Steps for the First Split
1. Choose the subsystems and their names.
2. Move the current project into the folder of the first subsystem with `git mv`: its `docs/`, source and tests. Relative links inside the moved tree keep working.
3. Give the subsystem its own `CLAUDE.md`, and create the top level from a fresh copy of the template: vision, architecture with the subsystems and their interfaces, and glossary.
4. Lift records that describe the whole system to the top level with `/new-record ... replaces <ID>`: the new top-level record replaces the old one, and the old record's `Replaced by` points up. Candidates: cross-cutting REQs and ADRs, FTRs that span subsystems, and shared DEPs. TDRs, SPKs and INCs stay where they are, since they record what happened; new ones go to the level they concern. Move the system-wide parts of the vision, the glossary and the open questions by hand.
5. Fix the references that the move changed: paths in code and test comments get the subsystem folder in front, and references to lifted records point to the top level. Also check links from the moved tree to files outside it, e.g. to the root `CLAUDE.md`: they still resolve, but now point to the subsystem's file.
6. Start further subsystems from copies of the template.

## Open Points
- The skills use paths relative to the repository root. They either need to be run from inside a subsystem folder, or need to learn which level they work on.
- `/check-requirements` could check that subsystem budgets add up to the top-level budget; it does not do that yet.
- `/new-record` numbers per folder; it does not yet know about subsystem folders.
- Whether each subsystem keeps its own `.claude/skills`, or all share the ones at the root. Claude Code loads the `CLAUDE.md` and `.claude/skills` of a subfolder when working inside it, preferring the closest ones, so a subsystem can override a shared skill where it needs to.
- `/check-requirements` only reads the REQs of one level, so it misses contradictions between a subsystem REQ and a top-level REQ, e.g. on licensing or security.
- A top-level FTR is `Implemented` once the work packages in all affected subsystems are `Done`, but `/implement` and `/new-record` only update FTRs in their own tre