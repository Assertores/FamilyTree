---
name: check-docs
description: Checks the documentation in docs/ for broken links, links to replaced records, records missing from their README, leftover placeholders, mismatched IDs, broken HTML comments, and terms missing from the glossary. Use when the user asks to check, lint, validate or clean up the documentation.
argument-hint: "[README topic, optional]"
---

# Check Docs

Find what slowly breaks as the documentation grows. Only report; change nothing unless the user asks for fixes afterwards.

Read every Markdown file in `docs/`. Links and placeholders inside HTML comments and code blocks are templates, not content: ignore them, except for the comment check below. Templates (`*_template.md`) are only checked for broken comments. If arguments name a README topic, only check the records listed under it and the documents they link to.

## 1. Structure

| Check | Severity |
| ---: | :--- |
| A relative link points to a file that does not exist, or to a heading that does not exist in that file | Error |
| A `-->` inside what is meant to be an HTML comment (e.g. a Mermaid arrow in a commented-out template) closes the comment early | Error |
| The `ID` in a record's header does not match its file name, e.g. `REQ 0007` in `req_0007.md` | Error |
| A link points to a record whose `Replaced by` names a different record | Warning |
| A record (`<prefix>_<number>.md`) is not listed in its folder's README | Warning |
| Two questions in `docs/open_questions.md` have the same `OQ` ID | Error |
| A header field still holds a placeholder starting with `#`, e.g. `Author: #NAME` | Warning |
| An object page in `docs/operations/objects/` has a `Last reviewed` date more than six months old | Warning |
| A row in the Contacts table of an object page has an empty Person or Contact column | Warning |
| A container in `docs/architecture.md` has no object page, while `docs/operations/objects/` has object pages for others | Warning |

## 2. Content

- **Glossary:** terms that have a specific meaning in this project, are used in several documents, and are missing from `docs/glossary.md`. Also terms used with different meanings in different documents.
- **Requirement Statements:** definitions or links inside a `## Requirement Statement` section; these are not allowed.
- **Features:** REQs that clearly constrain a feature but are missing from its Requirements table.
- **Incidents:** INCs in the state `Reviewed` without Consequences, INCs whose Timeline lacks one of the events **Started**, **Detected**, **Mitigated** or **Resolved**, and INCs in the state `Resolved` whose **Resolved** event is more than two weeks ago.
- **Open questions:** questions under Open in `docs/open_questions.md` that are already answered somewhere in the documentation, e.g. by a `Concluded` spike.

Only report what you are confident about. No findings is a good result.

## 3. Report

Group the findings by file, errors first. For each finding, say in one line what is wrong and what the fix would be. Ask whether to apply the fixes.
