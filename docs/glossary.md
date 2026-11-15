# Glossary
Terms with a specific meaning in this project, in alphabetical order. Link to a term from other documents with `[Term](glossary.md#term)`.

## Terms
<!-- TEMPLATE
### TERM
DEFINITION
-->
### Application
A program built on top of the [Library](#library), e.g. the [Browsing Application](#browsing-application) or the [Data Entry Tool](#data-entry-tool).

### Browsing Application
The [Application](#application) with which family members explore a [Data Set](#data-set): they search for [Persons](#person), see their [Details](#detail), look at the [Family Tree](#family-tree) around them, look at their [Pictures](#picture) and listen to their [Recording](#recording).

### Child
[Person](#person) B is a Child of Person A if a [Relation](#relation) from A to B has the [Relation Type](#relation-type) Strictly Lower, or a Relation from B to A has the Relation Type Strictly Higher.

### Context
One independent instance of the [Library](#library) that holds one [Data Set](#data-set). Everything the Library stores belongs to a Context.

### Data Entry Tool
The [Application](#application) with which a data collector fills in the [Details](#detail) of a [Person](#person) in a form and saves them as a [Person File](#person-file).

### Data File
A [Person File](#person-file) or a [Relations File](#relations-file).

### Data Folder
A folder that contains one [Person Folder](#person-folder) for every [Person](#person), and a [Relations File](#relations-file).

### Data Set
All [Persons](#person) and [Relations](#relation) read into one [Context](#context), from one or more [Data Folders](#data-folder).

### Default Platform
The [Platform](#platform) the [Library](#library) brings along for Windows and Linux. It is used for every part of the Platform that an [Application](#application) does not replace, i.e. for every [Question](#question) the Application leaves to the default.

### Detail
One piece of information about a [Person](#person) from a [Person File](#person-file): title, first names, title of nobility, last names, gender, date of birth, place of birth, date of death, place of death, professions, [Places of Residence](#place-of-residence), or remarks.

### Distance
The smallest number of steps from a [Person](#person) to a [Family](#family), where a step goes from one Family to another through a Person who is a member of both. The Families the Person is a member of have the Distance 0.

### Error
A failure that kept a call of the [Library](#library) from doing what was asked, e.g. conflicting [Generations](#generation) or an unknown [Person Id](#person-id).

### Family
A group of [Parents](#parent) together with all the [Children](#child) they have in common. The Parents and the Children are the members of the Family. A [Person](#person) is a member of the Family they are a Child in, and of every Family they are a Parent in.

### Family Graph
All [Persons](#person) that are connected to each other by a chain of [Parents](#parent) and [Children](#child). A [Data Set](#data-set) can consist of several Family Graphs that are not connected to each other.

### Family Tree
The selected [Person](#person), and all [Families](#family) within a given [Distance](#distance) of them together with all their members, ordered by [Generation](#generation). The Family Tree always contains the selected Person, also if they are a member of no Family.

### Format Version
A UUID in every data file that states which version of the file format the file is written in. Every version of the format has a UUID of its own, so that versions can develop separately from each other without conflicting version numbers. The [Person File](#person-file) and the [Relations File](#relations-file) are different formats, each with UUIDs of its own.

### Generation
The level of a [Person](#person) in their [Family Graph](#family-graph), counted from 0 at the highest level downwards, i.e. towards the [Children](#child). A higher Generation has a smaller number, a lower Generation a larger one.

### Library
The C library at the core of the project, which reads [Data Sets](#data-set) and answers questions about them.

### Media
The [Pictures](#picture) and the [Recording](#recording) of a [Person](#person).

### Parent
[Person](#person) A is a Parent of Person B if B is a [Child](#child) of A.

### Partner
Two [Persons](#person) are Partners if they have at least one [Child](#child) in common. A [Relation](#relation) with the [Relation Type](#relation-type) Unrestricted alone does not make two Persons Partners.

### Person
A member of the family, described by a [Person File](#person-file) and connected to others by [Relations](#relation).

### Person File
A JSON file that holds the [Person Id](#person-id), the [Format Version](#format-version), the [Details](#detail) and the references to the [Media](#media) of one [Person](#person).

### Person Folder
A folder inside a [Data Folder](#data-folder) that holds the [Person File](#person-file) and the [Media](#media) of one [Person](#person).

### Person Id
An integer that identifies a [Person](#person) within a [Data Folder](#data-folder). The [Person File](#person-file) and the [Relations File](#relations-file) refer to a Person by it.

### Picture
An image file of a [Person](#person), referenced by the [Person File](#person-file).

### Place of Residence
A place where a [Person](#person) lived, with an optional start date and end date.

### Platform
The replaceable part through which the [Library](#library) accesses the operating system: listing folders, checking whether files exist, and reading files. The Library reaches it by asking [Questions](#question).

### Question
A request the [Library](#library) makes to the [Application](#application) while it creates a [Context](#context), e.g. for the folders in a path or the content of a file. The Application answers it itself, or leaves the answer to the [Default Platform](#default-platform).

### Recording
An audio file of a [Person](#person), referenced by the [Person File](#person-file), e.g. the Person telling a story about their life.

### Relation
An entry in the [Relations File](#relations-file) that connects two [Persons](#person), with a [Relation Type](#relation-type), and optionally a [Relationship](#relationship), a start date and an end date. A Relation goes from the first Person to the second.

### Relation Type
The kind of a [Relation](#relation) with regard to [Generations](#generation): Unrestricted if it does not restrict the Generations, e.g. a marriage; Strictly Lower if the second [Person](#person) is in a lower Generation than the first, e.g. a child; Strictly Higher if the second Person is in a higher Generation than the first, e.g. a parent.

### Relations File
A CSV file in a [Data Folder](#data-folder) that lists the [Relations](#relation) between the [Persons](#person) of that Data Folder.

### Relationship
A free text that describes a [Relation](#relation) from the point of view of the first [Person](#person), e.g. "stepfather".

### Sibling
Two [Persons](#person) are Siblings if they have at least one [Parent](#parent) in common.

### Step
Something the [Library](#library) did, reported to the [Trace](#trace) to follow its flow, e.g. reading a [Person File](#person-file).

### Subtrace
A part of the [Trace](#trace) that covers one task of the [Library](#library), e.g. reading a [Person File](#person-file), and shows its outcome, so that a successful task can be skipped without reading it.

### Trace
A record of the steps the [Library](#library) takes and the failures it runs into, handed to the Library by an [Application](#application). If the Application hands none over, the Library uses a Trace that discards every report.

### Warning
A problem in the data that the [Library](#library) worked around, while the call still did what was asked, e.g. a skipped [Person File](#person-file) or an unknown key.
