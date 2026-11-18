# CLAUDE.md

## Project
What this project is and why it exists is described in [docs/vision.md](docs/vision.md); its structure in [docs/architecture.md](docs/architecture.md); how the code is written in [docs/conventions.md](docs/conventions.md). The index of all documentation is [docs/README.md](docs/README.md).

## Workflow
Each step builds on the documents of the previous ones. Dependencies (DEP) are recorded whenever they become clear, open questions are collected throughout, and a spike (SPK) is run whenever an open question blocks progress, most often before step 2 and during step 3.

Steps 2 to 6 infer as many requirements as possible from the vision, and write as many tests as possible from them before any code exists. The gaps in the requirements that the implementation uncovers are closed in step 8, work package by work package.

| Step | Work | Skills |
| ---: | :--- | :--- |
| 1 | Vision | |
| 2 | Glossary, as many requirements as can be inferred from the vision, open questions | `/new-record`, `/check-requirements` |
| 3 | Architecture and ADRs | `/new-record` |
| 4 | Handover: review and complete [docs/workspace/handover.md](docs/workspace/handover.md) | |
| 5 | Unit tests from the requirements. The interfaces invented for them are collected in `docs/workspace/interface_draft.md` | `/requirements-to-tests` |
| 6 | Consolidation: the interface draft becomes new ADRs and stubs, then the draft is deleted. Open questions are addressed | `/new-record` |
| 7 | Implementer's notes: work packages and notes in [docs/workspace/implementers_notes.md](docs/workspace/implementers_notes.md) | `/foreman` |
| 8 | Implementation, one work package per session: settle the structure of the code with the user on an outline in `docs/workspace/` and record it in an ADR, close the gaps in the requirements the package uncovers with new records and their tests, clear up uncertainties with the user, then implement. The whole test suite runs, to catch side effects on finished packages | `/implement`, `/new-record`, `/check-requirements`, `/requirements-to-tests`, `/log-session` |
| 9 | Features, once the biggest uncertainties are cleared, during the implementation. Each new feature is added to the work packages that implement it | `/new-record`, `/check-scenarios` |
| 10 | End-to-end tests from the feature scenarios, once a feature is `Implemented` | `/scenarios-to-tests` |

`/check-docs` can be used at any step.

During steps 1 to 3, before a session ends, add to [docs/workspace/handover.md](docs/workspace/handover.md) what came up in the session that matters for the implementation but is not in the documents: pitfalls, testing details, the intended implementation order. Do not repeat what the documents already say.

Outlines live in `docs/workspace/outline_<part>.<extension>`: incomplete code that shows the structure of a part of the code before it is implemented. They are documents, not code, and are deleted once the code follows the ADR that records them.

Spike code lives in `spikes/spk_<number>/`. It is never part of the product, and the rules for tests do not apply to it. Its inputs and observed outputs can become test data. When a spike is `Concluded`, its findings become records with `/new-record`, and its open questions move to Answered with a link to the spike.

The tests are derived from the documentation. If a test looks wrong, do not change it to make the code pass: point it out, and fix the documentation first if that is where the error is.

When a document turns out to be out of date, point it out and suggest the update; do not quietly work around it.

## Records
| Prefix | Folder | Content |
| ---: | :--- | :--- |
| REQ | `docs/requirements/` | what the system must do or comply with |
| ADR | `docs/decisions/` | how and why parts of the system are built the way they are |
| FTR | `docs/features/` | user story, constraining requirements, Given/When/Then scenarios |
| DEP | `docs/dependencies/` | external software, Runtime or Development scope |
| TDR | `docs/issues/` | technical debt taken on knowingly |
| SPK | `docs/spikes/` | experiments that answer open questions: question, approach, findings |

- Create records with `/new-record`. `<prefix>_0000.md` is an example and `<prefix>_template.md` the template; neither is a real record.
- A record's meaning is never edited. A change is a new record that replaces the old one (`/new-record ... replaces <ID>`): REQ, ADR, FTR, DEP and SPK link both ways through `Replaced by` and `Replaces`. An FTR that is dropped without a successor gets the state `Rejected`. Replacements can split or merge records. TDRs, and SPKs while `In Progress`, are edited directly.
- Requirement Statements use [RFC 2119](https://datatracker.ietf.org/doc/html/rfc2119) keywords and contain no definitions and no links. Terms are defined in [docs/glossary.md](docs/glossary.md).

## Writing Documentation
- Keep the `<!-- -->` hint comments in filled documents. Hints say what goes into a section, never why the section exists or what is done with it.
- Remove a `<!-- TEMPLATE` block once its section is filled, unless more entries are expected (e.g. table rows, Discussions).
- Never put anything containing `-->` inside an HTML comment, e.g. a Mermaid arrow: it closes the comment. Hide such templates in `<div hidden>` instead.
- Diagrams are Mermaid diagrams. Choose the kind by what the diagram shows: a sequence diagram for who does what in which order, an entity relationship or class diagram for the parts and what each of them holds, a state diagram for states and what changes them, and a flowchart only for a decision process or a pipeline that data passes through, in which every arrow means the same.
- Use a table when entries share the same attributes, with the first column right-aligned. Use bullet points for single statements.
- Never skip heading levels.
- Dates are `YYYY-MM-DD`. `Last reviewed` changes only when someone deliberately confirms that the whole document is still correct, not on every edit.
- Examples (`<prefix>_0000.md`, and the example object page) describe the template itself, not a made-up system.
- Examples within a record that show how a design works, e.g. in a diagram or a sample Trace, use made-up data, not the project's own, so that they are not mistaken for its actual behavior.
- Link to related documents where they exist; do not force links that do not.
- READMEs only list links, each with a short description of the document type. Records are grouped by topic; dependencies by Runtime and Development.
