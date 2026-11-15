---
Author: #NAME
Created at: #YYYY-MM-DD
Last reviewed: #YYYY-MM-DD
---
# <!-- PROJECT NAME --> Incident Procedure
How an incident is declared, led, ended and reviewed. The defaults below are a starting point; adapt them to the team. Not tried on a real project yet: adjust this procedure, the object pages and the INC template with what the first incidents teach.

## Roles
<!-- Who does what during an incident. One person may hold several roles, except the Investigator, who holds no other role. -->
| Role | Responsibility |
| ---: | :--- |
| Incident Manager | leads the incident: declares it, assigns the roles, decides, e.g. on a rollback, and ends it |
| Investigator | finds and fixes the cause: reads code, logs and metrics, proposes mitigations to the Incident Manager. Can be several people |
| Communicator | handles everyone else: posts status updates at a fixed interval, answers questions, collects new information and offers of help, and passes on to the Investigator only what is relevant |
| Scribe | the incident chat: every finding, decision and action is posted there, so the chat becomes the timeline |

- In a small team, the Incident Manager can also be the Communicator. The Investigator never takes on another role, because the investigation needs their full attention.
- Roles are handed over explicitly in the incident chat, e.g. at a change of shift.

## Severity Levels
<!-- How serious an incident is, and what that means for the response, e.g. who is called and how often updates are posted. -->
<!-- TEMPLATE
| Severity | Criteria | Response |
| ---: | :--- | :--- |
| SEV 1 | e.g. the service is down for all users, or data is lost | e.g. call the on-call rotation at any time, updates every 30 minutes |
-->

## Channels
<!-- Where an incident is communicated, and who reads it. -->
<!-- TEMPLATE
| Channel | Audience | Purpose |
| ---: | :--- | :--- |
| NAME | e.g. responders, stakeholders, users | e.g. incident chat, status page, stakeholder updates |
-->

## Declaring an Incident
- Anyone who suspects an incident may declare one. Declaring an incident that turns out to be harmless costs little; declaring one too late costs a lot.
- The Incident Manager opens the incident chat, assigns the roles and sets the severity.
- The object page of the affected container in [Object Pages](objects/README.md) is the first stop for everyone involved.

## During the Incident
- Mitigation comes before understanding: stop the damage first, e.g. with a rollback or a feature flag listed on the object page, and find the cause afterwards.
- Every finding, decision and action goes into the incident chat with its time, even when it was said in a call.
- Times are `YYYY-MM-DD HH:MM` in UTC.

## Hotfixes
- A fix during an incident may skip the usual order of documentation, then tests, then code. The Incident Manager decides.
- Every shortcut is posted in the incident chat, e.g. a test that was disabled or changed, or a requirement the fix contradicts. The post-mortem brings the documentation and the tests in line with the code afterwards, or the code back in line with them.
- What the post-mortem does not resolve becomes a TDR.

## Ending the Incident
- The Incident Manager ends the incident once the service behaves normally again, and says so in the incident chat and in the channels.
- The INC record is created at the latest now, from the incident chat, with `/new-record inc <title>`.

## Post-Mortem
- Held within a few days of the incident, while memories are fresh, with everyone who held a role.
- Blameless: it asks what made the failure possible, not who caused it. People acted on what they knew at the time.
- It fills the INC record and classifies where the chain from requirement to test to code broke:

| Break | Meaning | Response |
| ---: | :--- | :--- |
| Missing requirement | no REQ covered the behavior | new REQ, its tests, then the fix |
| Wrong requirement | a REQ asked for the wrong behavior | a REQ that replaces it, updated tests, then the fix |
| Missing or wrong test | the REQ was right, but no test caught the violation | the test fixed or added from the REQ |
| Wrong assumption | requirements and tests were right, but the environment behaved differently, e.g. a dependency or the load | new ADR or DEP, or a correction of the assumptions in the vision |

- Every consequence becomes a record, a test or a work package, and is linked in the INC record. The object page of the affected container is updated with what the incident taught.
- Security incidents, or incidents involving personal data, get only a short summary in the INC record, with a link to a restricted report.
