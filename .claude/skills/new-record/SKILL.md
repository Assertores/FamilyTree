---
name: new-record
description: Creates a new documentation record (REQ, ADR, FTR, DEP, TDR, SPK or INC) from its template with the next free number, today's date and the author, and lists it in the folder's README. Can also create a record that replaces an existing one. Use when the user wants to add a new requirement, decision, feature, dependency, technical debt, spike or incident record, or to change, revise or replace an existing REQ, ADR, DEP, FTR or SPK.
argument-hint: "<req|adr|ftr|dep|tdr|spk|inc> <title> [replaces <IDs>] [content to draft]"
---

# New Record

## 1. Find the record type

| Type | Folder | Template | Initial state | Back link field |
| ---: | :--- | :--- | :--- | :--- |
| REQ | `docs/requirements/` | `req_template.md` | none | `Replaces` |
| ADR | `docs/decisions/` | `adr_template.md` | none | `Replaces` |
| FTR | `docs/features/` | `ftr_template.md` | Proposed | `Replaces` |
| DEP | `docs/dependencies/` | `dep_template.md` | none | `Replaces` |
| TDR | `docs/issues/` | `tdr_template.md` | Identified | none |
| SPK | `docs/spikes/` | `spk_template.md` | In Progress | `Replaces` |
| INC | `docs/operations/incidents/` | `inc_template.md` | Open | none |

If the type or the title is missing from the arguments, ask for it.

**Replacement:** if the user wants to change an existing REQ, ADR, DEP or FTR, or a SPK that is no longer `In Progress` (e.g. "replaces REQ 0003", or "REQ 0003 needs to change"), the new record replaces the old one. Records are never edited to change their meaning; a change is always a new record. TDRs, INCs, and SPKs while `In Progress`, are edited directly instead.

A replacement can be:

- **one to one:** one new record replaces one old record.
- **a split:** several new records replace one old record. Create them in one go, with consecutive numbers; ask for the titles if they are not given.
- **a merge:** one new record replaces several old records.

## 2. Create the file

1. **Number:** the highest number used for this record type plus one, four digits (`0000` is the example, so the first real record is `0001`). Look at the folder in the working tree and on every local and remote branch, so that records created on other branches are not numbered twice, e.g. for REQs:
    ```
    git for-each-ref --format='%(refname)' refs/heads refs/remotes | xargs -I{} git ls-tree --name-only {} docs/requirements/
    ```
2. **File:** copy the template to `<prefix>_<number>.md`.
3. **Header:**
    - `ID`: e.g. `REQ 0007`.
    - `State`: the initial state from the table above, if the template has a state.
    - `Scope` (DEP only): ask whether it is Runtime or Development, unless the arguments make it clear. For a replacement, take it from the old record.
    - `Severity` (INC only): take it from the arguments or the incident chat, or leave the placeholder.
    - `Replaced by`: an empty list.
    - Back link field `Replaces`: for a replacement, one link per old record, e.g. `- "[REQ 0003](req_0003.md)"`. Otherwise an empty list.
    - `Author`: the output of `git config user.name`.
    - `Created at` and `Last reviewed`: today's date as `YYYY-MM-DD`.
4. **Title:** replace `<!-- TITLE --> TEMPLATE` with the title. For a replacement without a new title, use the old record's title.
5. **Body:** keep all hint comments.
    - New record: if the arguments contain content, draft the sections from it and leave the rest empty.
    - Replacement: start from the old records' content and apply the change the user described. In a split, each new record takes only the part of the old content it is responsible for, and together they must cover everything the old record said, unless the user dropped something on purpose. Explain the change in the Context section, if the record has one. Do not copy the old record's Discussions.
    - Never invent content the user did not give.

## 3. Update the old record (replacement only)

1. Add one link per new record to the old record's `Replaced by` list, e.g. `- "[REQ 0009](req_0009.md)"`. Remove placeholders and links to the record itself from that list first.
2. Change nothing else in the old record.

## 4. Update the README

Add `- [<ID>](<file>) <title>` to the folder's `README.md`:

- Under the matching `## <topic>` heading. If no topic fits, ask the user whether to create a new topic or which one to use. A replacement goes under the old record's topic.
- For dependencies, under `## Runtime` or `## Development` according to the scope.
- Replacement: append ` (replaced by [<new ID>](<new file>))` to each old record's README entry, listing all new records separated by commas. If the entry already has such a note, extend it instead of adding a second one.

## 5. Add a feature to the work packages (FTR only)

Skip this step if the Work Packages table in `docs/workspace/implementers_notes.md` is still empty.

1. Suggest which work packages implement the feature, based on its scope and its Requirements table, and ask the user to confirm. If the feature needs work that no package covers, suggest a new package row with the state `Open`, placed after the packages it depends on, and its notes under Notes per Work Package.
2. Add a link to the FTR to the scope of each confirmed package. For a replacement, replace the links to the old FTR instead.
3. If the feature needs new work in a package that is `Done`, set that package back to `Open` and note what is missing in its notes.
4. Set the FTR's `State`: `Implemented` if every package that implements it is `Done`, `In Progress` if one of them is `In Progress`, otherwise `Planned`.

## 6. Report

Name the created file and the README entry in one or two lines. For a replacement, also list the other documents that link to the old record (e.g. FTRs listing a replaced REQ), and for a replaced FTR the tests that reference its scenarios, and ask whether to point them to the new record.

Then point out what else the new record may affect, and suggest the changes; make them only once the user agrees:

- ADR or DEP: whether `docs/architecture.md` or `docs/conventions.md` describes something the record changes.
- INC: the object pages in `docs/operations/objects/` of the affected containers, e.g. a new failure mode or a missing shut-off. Once the post-mortem is done, the records, tests and work packages its consequences call for.
- REQ: the wording is often refined with the user first. Only once the user says the requirement is settled, suggest checking it against the other requirements with `/check-requirements <ID>` and writing its tests with `/requirements-to-tests <ID>`, unless `/implement` already does both.
- Any record: terms it uses with a specific meaning that are missing from `docs/glossary.md`.
