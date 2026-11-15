---
name: check-requirements
description: Formalizes the Requirement Statements of all REQ documents as deontic modal logic, then proves contradictions between them and requirements that already follow from others. Use when the user asks to check, verify or analyze the requirements for consistency, contradictions, conflicts or redundancy.
argument-hint: "[REQ IDs or a README topic to focus on, optional]"
---

# Check Requirements

Analyze the requirements in `docs/requirements/` for **contradictions** and **redundancies**, and present every finding as a formal proof. Never edit the requirement documents; only report.

## 1. Collect

1. Read every `docs/requirements/req_*.md` except `req_template.md` and the example `req_0000.md`.
2. From each, take the `ID` from the header and the text of the `## Requirement Statement` section. Ignore HTML comments.
   Requirement Statements must not contain definitions or links. If one does, formalize it without them and report it as a rule violation after the findings.
3. Skip requirements whose `Replaced by` field links to a *different* REQ: they are no longer in force. Links to the document itself are placeholders and are ignored.
4. If arguments name specific REQ IDs, still formalize all requirements, but only report findings that involve the named ones. If they name a topic of `docs/requirements/README.md`, only formalize the REQs listed under it, and say in the report that contradictions with REQs outside the topic were not checked.

## 2. Formalize

Translate each statement into Standard Deontic Logic (SDL, modal system KD). Use these operators:

| RFC 2119 keyword | Formula | Reading |
| ---: | :--- | :--- |
| MUST, SHALL, REQUIRED | O(φ) | φ is obligatory |
| MUST NOT, SHALL NOT | O(¬φ) | φ is forbidden |
| MAY, OPTIONAL | P(φ) | φ is permitted, where P(φ) ≡ ¬O(¬φ) |
| SHOULD, RECOMMENDED | R(φ) | φ is recommended (weak obligation) |
| SHOULD NOT, NOT RECOMMENDED | R(¬φ) | φ is discouraged |

Rules for the translation:

- **Atoms:** define one propositional atom per distinct condition or behavior, with a short name and its meaning in plain words, e.g. `logged_in`: "the user is logged in". Use terms from `docs/glossary.md` where they exist. Two statements describing the same thing in different words get the same atom; say so explicitly.
- **Conditions:** "if/when X, … SHALL Y" becomes `X → O(Y)`.
- **One statement, several requirements:** split it into several formulas, numbered `REQ 0003a`, `REQ 0003b`, …
- **Domain facts:** facts not stated in any REQ but true in every situation, and needed for a proof (e.g. "a reply within 100 ms is also a reply within 200 ms", or "a file cannot be both read-only and writable"). They become explicit premises `A1`, `A2`, … with a one-sentence reason. Only add facts that are uncontroversial; never hide reasoning inside a step.
- **Situations:** conditions that can be true at some moment, needed to trigger conditional requirements (e.g. "the user is logged in and the document is read-only"). They become explicit premises `S1`, `S2`, … with how likely they are.
- **Ambiguity:** if a statement allows several readings, formalize the most likely one and list the others in the report. Do not guess silently.

## 3. Derive

Reason with these rules only. They are for your own derivation; the report does not name them.

| Rule | Statement |
| ---: | :--- |
| K | O(φ → ψ) → (O(φ) → O(ψ)) |
| D | O(φ) → ¬O(¬φ), equivalently O(φ) → P(φ) |
| Nec | from ⊢ φ infer ⊢ O(φ) |
| RE | from ⊢ φ ↔ ψ infer O(φ) ↔ O(ψ) |
| MP | from φ and φ → ψ infer ψ |
| PL | any propositional tautology |
| Def P | P(φ) ↔ ¬O(¬φ) |

Nec applies only to theorems: tautologies and domain facts. Never apply it to REQ formulas or situations; "the user is logged in" being true does not make it obligatory.

Look for the following. Finding nothing is a valid and good result: only report what you can actually prove from the requirements and well-founded assumptions. Never stretch an interpretation or invent an unlikely assumption to produce a finding.

- **Contradictions:** a derivation of ⊥ from the premises (all REQ formulas plus domain facts and situations). Typical patterns: O(φ) and O(¬φ); O(φ) and P(¬φ); O(φ) and O(ψ) where φ ∧ ψ is impossible by some domain fact. If a contradiction needs a condition to be true (e.g. `X → O(φ)` and `Y → O(¬φ)`), it only arises when `X ∧ Y` holds: add that as a situation and judge how likely it is. If the condition is impossible, there is no contradiction.
- **Tensions:** an R formula against an O formula, e.g. R(φ) and O(¬φ). These are not contradictions in SDL; report them separately.
- **Redundancies:** a requirement `REQ n` whose formula is derivable from the *other* premises alone. Prove it without using `REQ n`.

SDL produces false contradictions for requirements that say what to do when another requirement is violated (e.g. "the backup SHALL NOT fail" and "if the backup fails, the system SHALL notify the admin"). Do not report a contradiction that only arises from such a pair.

## 4. Report

Write every finding as a short proof in plain language. Translate each derived formula back into an RFC 2119 sentence in quotes, using the wording of the requirements. Do not print formulas or rule names.

```
## Contradiction 1: REQ 0001, REQ 0003, REQ 0012
1. From REQ 0012 and REQ 0003 it follows that "when the user is logged in, the system SHALL show the edit button".
2. From REQ 0001 it follows that "when the document is read-only, the system SHALL NOT show the edit button".
3. Assumption: the user is logged in and the document is read-only at the same time (likely: any logged-in user can open a read-only document).
4. From 1, 2 and 3 it follows that the system SHALL and SHALL NOT show the edit button at the same time. This is a contradiction. ∎

## Redundancy 1: REQ 0007
1. From REQ 0002 it follows that "...".
2. From 1 and REQ 0005 it follows that "...".
3. This is exactly REQ 0007. ∎

## Tension 1: REQ 0004, REQ 0009
1. ...

```

- Every step says what it follows from: REQ IDs, earlier step numbers, or an assumption.
- Every assumption states what is assumed and how likely it is (certain, likely, unlikely, or unknown), with a short reason.
- List contradictions first, then redundancies, then tensions.
- After the findings, list the ambiguous statements with their alternative readings, and the statements that contain definitions or links.
- If nothing is found, say so, and list how each requirement was read, so the interpretation can be checked.
