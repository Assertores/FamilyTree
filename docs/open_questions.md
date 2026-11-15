# Open Questions
Questions about the project that are not answered yet. Once a question is answered, record the answer where it belongs, for example in a REQ or the vision, and move the question to Answered.

## Open
<!-- One entry per question, with the next ID after the highest one in this document, Open and Answered together. -->
<!-- TEMPLATE
- **OQ 0001** QUESTION
-->
- **OQ 0006** (stretch goal) Sensitive Details are kept in a separate Data Folder and combined with the openly shared one by the Library. How does the Library find which Person in one Data Folder is the same as a Person in another? Which value wins if both describe the same Detail?
- **OQ 0007** (stretch goal) Data Sets are expected to diverge, so the same Person Id can mean different Persons in different Data Folders. How does the Library tell them apart when the Data Folders are combined?
- **OQ 0008** (stretch goal) The same Person Id in two Person Folders of one Data Folder is an error in the data. What does the Library do with it?
- **OQ 0019** How does the Library tell steps, warnings and errors apart, in the Trace and in what it returns to the Application? To be decided in the architecture.

## Answered
<!-- Answered questions with their ID unchanged, and a link to where the answer was recorded. -->
<!-- TEMPLATE
- **OQ 0001** QUESTION Answered in [REQ 0000](requirements/req_0000.md).
-->
- **OQ 0001** In which language do the Applications talk to their users? Answered in [REQ 0046](requirements/req_0046.md) and [REQ 0004](requirements/req_0004.md).
- **OQ 0002** Is the "Geschichtenstammbaum" a feature of its own? Answered in the [Vision](vision.md#should-have-target) and [REQ 0012](requirements/req_0012.md).
- **OQ 0003** Is the command line interface still a goal? Answered in the [Handover](workspace/handover.md#testing-notes).
- **OQ 0004** Is the viewer for recorded Traces part of the product? Answered in the [Handover](workspace/handover.md#other).
- **OQ 0005** How many Persons will the real Data Set hold? Answered in the [Vision](vision.md#assumptions).
- **OQ 0009** What does the Library do with a Relation whose Relation Type is empty or unknown? Answered in [REQ 0008](requirements/req_0008.md).
- **OQ 0010** Is `relations_extra.csv` still used? Answered in the [Handover](workspace/handover.md#testing-notes) and the [Vision](vision.md#could-have-stretch).
- **OQ 0011** Are two Persons with an Unrestricted Relation Partners? Answered in the [Glossary](glossary.md#partner) and [REQ 0020](requirements/req_0020.md).
- **OQ 0012** How does a search compare a given Detail with the Details of a Person? Answered in [REQ 0016](requirements/req_0016.md).
- **OQ 0013** What Generation do Persons get that are not connected by any chain of Parents and Children? Answered in [REQ 0021](requirements/req_0021.md) and [REQ 0045](requirements/req_0045.md).
- **OQ 0014** How does the Format Version look in the future? Answered in the [Glossary](glossary.md#format-version) and [REQ 0002](requirements/req_0002.md).
- **OQ 0015** Should the Data Entry Tool create the Person Folder and copy the Media into it? Answered in the [Vision](vision.md#could-have-stretch).
- **OQ 0016** Should an Application let the data collector enter Relations? Answered in the [Vision](vision.md#could-have-stretch).
- **OQ 0017** Does a search ignore upper and lower case? Left to the implementation, see the [Handover](workspace/handover.md#other).
- **OQ 0018** Which controls does the story player need beyond pausing and continuing? Left to the implementation, see the [Handover](workspace/handover.md#other).
