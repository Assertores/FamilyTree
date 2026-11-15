---
name: check-scenarios
description: Checks the Given/When/Then scenarios of the feature records (FTR) against the Requirement Statements of the REQs, and proves where a scenario violates a requirement. Use when the user asks to check, verify or review features or scenarios against the requirements.
argument-hint: "[FTR IDs or a README topic to focus on, optional]"
---

# Check Scenarios

Find scenarios whose behavior is not allowed by the requirements, and requirements that constrain a feature without being listed in it. Report first; change nothing unless the user asks for fixes afterwards.

## 1. Collect

1. Read every `docs/features/ftr_*.md` except `ftr_template.md` and the example `ftr_0000.md`. Skip features in state `Rejected`, and features whose `Replaced by` links to a different FTR. If arguments name FTR IDs, only check those; if they name a topic of `docs/features/README.md`, only the FTRs listed under it.
2. Read the requirements as described in sections 1 and 2 of `.claude/skills/check-requirements/SKILL.md`, and formalize them the same way.

## 2. Formalize the scenarios

For each scenario:

- **Given** becomes a situation: what is true before the action.
- **When** becomes the action that happens in that situation.
- **Then** becomes what the system does as a result.

Use the same atoms as for the requirements, so that "shows the edit button" in a scenario and in a REQ is the same statement.

## 3. Derive

Use the rules from section 3 of `.claude/skills/check-requirements/SKILL.md`, including its limits. Look for:

- **Violations:** a scenario in which the system does something a REQ forbids in that situation, or fails to do something a REQ requires in that situation.
- **Missing constraints:** a REQ that applies to a scenario's situation but is not listed in the feature's Requirements table.

Finding nothing is a valid and good result. Never stretch an interpretation to produce a finding.

## 4. Report

Write every violation as a short proof in plain language, in the same style as the check-requirements report:

```
## Violation 1: FTR 0003 "Wrong password", REQ 0008
1. In the scenario, the user has entered a wrong password three times.
2. Then the scenario says "the login form stays enabled".
3. From REQ 0008 it follows that "after three wrong passwords, the system SHALL lock the login form".
4. From 1, 2 and 3 it follows that the scenario does not do what REQ 0008 requires. ∎
```

After the violations, list the missing constraints as one line each: the feature, the REQ, and the scenario it applies to.

Then ask whether to fix the missing constraints. If the user agrees, add each missing REQ to the feature's Requirements table, with one line on how it constrains the feature, and update the feature's `Last reviewed` date.

Do not fix violations on your own: either the scenario or the requirement is wrong, and that is the user's decision. Ask which one to change, and change it only once the user has decided.
