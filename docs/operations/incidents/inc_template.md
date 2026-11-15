---
ID: #INC 0000
State: #Open, Resolved, OR Reviewed
Severity: #SEV 1
Author: #NAME
Created at: #YYYY-MM-DD
Last reviewed: #YYYY-MM-DD
---
# <!-- TITLE --> TEMPLATE
<!-- State: Open (the incident is running, or the record is being written), Resolved (the service behaves normally again), Reviewed (the post-mortem is done). Times are in UTC. -->
## Summary
<!-- Two or three sentences: what failed, for whom, for how long, and how it was mitigated. -->

<!-- ## Impact
only if it can be estimated -->
<!-- What users and the business noticed, in numbers where possible, e.g. failed requests, affected users, lost data, broken promises in the service level objectives. -->

## Roles
<!-- Who held which role from the incident procedure, and when roles were handed over. -->
<!-- TEMPLATE
| Role | Person |
| ---: | :--- |
| Incident Manager, Investigator, OR Communicator | NAME |
-->

## Timeline
<!-- The key events taken from the incident chat, and a link to the chat or its export. The events when the incident started, was detected, was mitigated and was resolved begin with **Started**, **Detected**, **Mitigated** and **Resolved**. -->
<!-- TEMPLATE
| Time | Event |
| ---: | :--- |
| YYYY-MM-DD HH:MM | **Started** what happened, was found, or was decided |
-->

## Detection
<!-- How the incident was noticed, e.g. an alert or a user report, how long that took, and whether the alert led to the right object page. -->

## Root Cause
<!-- What made the failure possible, and the contributing factors. Blameless: the conditions, not the people. -->

## Chain Break
<!-- Where the chain from requirement to test to code broke, as classified in the [incident procedure](../incident_procedure.md#post-mortem), with the REQs and tests involved. -->

## Consequences
<!-- The records, tests, work packages, and updates to object pages or the incident procedure that came out of the post-mortem. Shortcuts from hotfixes that stay become TDRs. -->
<!-- TEMPLATE
- [REQ 0000](../../requirements/req_0000.md) what it takes from the incident
-->

<!-- TEMPLATE
## See Also
 - [INC 0000](inc_0000.md) reason why this is related
-->

-----
## Discussions
<!-- TEMPLATE
### Topic
NAME
> COMMENT
-->
