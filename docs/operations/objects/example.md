---
Author: Andreas Edmeier
Created at: 2026-11-15
Last reviewed: 2026-11-15
---
# Example Object Page
## Description
<!-- Two or three sentences: what this container does, and what users notice when it fails. Link to its section in the architecture instead of repeating it. -->
This page treats the documentation of this template, the `docs/` folder, as if it were a container. When it fails, readers and skills find no guidance or the wrong guidance, and the records written from it come out wrong.

## Contacts
<!-- Who to call about this container, and how to reach them right away, through something that stays current. -->
| Role | Person | Contact | Reachable |
| ---: | :--- | :--- | :--- |
| Code owner | Andreas Edmeier | assertores@web.de | whenever the email is read |

## Access
<!-- Where the container runs, and how responders get to it: environments, permissions needed, emergency access, and links to dashboards and log searches. -->
- Runs wherever the repository is checked out; no permissions are needed beyond reading it.
- The index is [docs/README.md](../../README.md). Opened as an Obsidian vault, the links form a graph of all documents.

## Interfaces
<!-- What this container provides and what it uses, and what happens on the other side when one end fails. Link to the interface in the architecture. -->
| Interface | Direction | Other Side | When It Fails |
| ---: | :--- | :--- | :--- |
| Records | provides | the skills, e.g. `/new-record` and `/check-docs` | the skills create, number or check records wrongly |
| Examples | provides | readers | records are written in inconsistent ways |
| Templates | uses | `*_template.md` | new records start with the wrong sections |

## Shut-Offs
<!-- How to stop the damage: rollbacks, feature flags, kill switches, throttling, maintenance mode. What each one does to the rest of the system. -->
| Measure | How | Effect |
| ---: | :--- | :--- |
| Revert | `git revert` of the commit that broke the documentation | the documentation is back to the previous commit; everything else in that commit is undone too |

## Hazards
<!-- What responders must not do, or only with care, e.g. irreversible migrations, restarts that overload other systems, data that must not be copied. -->
- Do not rename a record to fix it: links from other records, code and tests break.
- Do not put `-->` inside an HTML comment: it closes the comment, and the rest of the template shows up as content.

## Error Signals
<!-- Metrics, logs and alerts that hint at errors in this container, with what they look like normally. -->
| Signal | Where | Normal | Hints At |
| ---: | :--- | :--- | :--- |
| Errors from `/check-docs` | its report | none | [Broken Links](#broken-links) |
| Discussions titled "I was linked here" | the examples | only in the examples themselves | [Broken Links](#broken-links) |
| An example that describes something made up | the `_0000.md` files and this page | every example describes the template itself | [Examples Read Differently](#examples-read-differently) |

## Failure Modes
<!-- One subsection per known way this container fails. Alerts link directly to the matching subsection. -->
### Broken Links
- **Symptoms:** `/check-docs` reports errors, or readers end up on an example instead of a real record.
- **Mitigation:** fix the link; if a whole commit broke many links, revert it.
- **Incidents:** none so far.

### Examples Read Differently
- **Symptoms:** an example describes a made-up system instead of the template itself.
- **Mitigation:** rewrite it to describe the template.
- **Incidents:** [INC 0000](../incidents/inc_0000.md)

## See Also
 - [INC 0000](../incidents/inc_0000.md) to see the example incident on this page
