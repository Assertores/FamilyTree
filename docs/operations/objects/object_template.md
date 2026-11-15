---
Author: #NAME
Created at: #YYYY-MM-DD
Last reviewed: #YYYY-MM-DD
---
# <!-- CONTAINER NAME --> TEMPLATE
## Description
<!-- Two or three sentences: what this container does, and what users notice when it fails. Link to its section in the architecture instead of repeating it. -->

## Contacts
<!-- Who to call about this container, and how to reach them right away, through something that stays current. -->
<!-- TEMPLATE
| Role | Person | Contact | Reachable |
| ---: | :--- | :--- | :--- |
| e.g. code owner, product manager, on-call, external partner | NAME OR rotation | paging link, team channel, email alias, OR directory entry | e.g. office hours, at any time, by support contract |
-->

## Access
<!-- Where the container runs, and how responders get to it: environments, permissions needed, emergency access, and links to dashboards and log searches. -->

## Interfaces
<!-- What this container provides and what it uses, and what happens on the other side when one end fails. Link to the interface in the architecture. -->
<!-- TEMPLATE
| Interface | Direction | Other Side | When It Fails |
| ---: | :--- | :--- | :--- |
| NAME | provides OR uses | CONTAINER OR EXTERNAL SYSTEM | what breaks, and for whom |
-->

## Shut-Offs
<!-- How to stop the damage: rollbacks, feature flags, kill switches, throttling, maintenance mode. What each one does to the rest of the system. -->
<!-- TEMPLATE
| Measure | How | Effect |
| ---: | :--- | :--- |
| NAME | command, link, OR procedure | what stops, and what else stops with it |
-->

## Hazards
<!-- What responders must not do, or only with care, e.g. irreversible migrations, restarts that overload other systems, data that must not be copied. -->
<!-- TEMPLATE
- HAZARD, and why
-->

## Error Signals
<!-- Metrics, logs and alerts that hint at errors in this container, with what they look like normally. -->
<!-- TEMPLATE
| Signal | Where | Normal | Hints At |
| ---: | :--- | :--- | :--- |
| NAME | dashboard, log query, OR alert | the usual value or pattern | the failure mode it points to |
-->

## Failure Modes
<!-- One subsection per known way this container fails. Alerts link directly to the matching subsection. -->
<!-- TEMPLATE
### FAILURE MODE
- **Symptoms:** what is observed
- **Mitigation:** what to do, usually a shut-off from above
- **Incidents:** [INC 0000](../incidents/inc_0000.md)
-->

<!-- TEMPLATE
## See Also
 - [Architecture](../../architecture.md) reason why this is related
-->
