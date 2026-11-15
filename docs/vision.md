---
Author: Andreas Edmeier
Created at: 2026-11-15
Last reviewed: 2026-11-15
---
# Family Tree Vision

## Vision Statement
<!-- One or two sentences: what the world looks like once this project succeeds. -->
The history of a family, its people, how they are related, their pictures and their recorded stories, stays readable and explorable for generations, even after the project has been shelved for decades and today's software, operating systems and online services are gone.

## Overview
<!-- Two or three sentences describing what the project concretely is, for someone hearing about it for the first time. -->
Family Tree is a small C library that reads a family's data from plain files, one folder per person with a JSON file and its images and audio recordings, plus a CSV file of relations, and answers questions about it: who someone is, who their parents, children, siblings and partners are, and which generation they belong to. On top of the library sit applications to browse the family as a list, a search and a tree, to look at pictures and listen to recorded stories, and to enter the data of a person without writing JSON by hand. The library is the focus of the project; the applications are a bonus on top of it. The project started under the name Geschichtenstammbaum, a family tree of stories, in which you go through the tree by listening to the stories of your ancestors.

## Problem
<!-- What pain exists today that this project addresses? -->
- The knowledge about a family's history lives in the heads of a few, often older, family members and in scattered photos and documents. Once they are gone, it is lost.
- Recorded stories, the voice of a grandparent telling about their life, are rarely kept together with the family tree they belong to.
- Genealogy software and online platforms keep the data in their own formats or on their own servers. Data entered today can become unreadable when the program no longer runs on a new operating system, or when the company or service disappears.
- Some information about a family is sensitive and must not be shared openly, while the rest should be available to the whole family.
- The people collecting the data are usually not programmers. They need a way to enter data that does not require editing files by hand, while the data itself must stay in a format a person can still read without any program.

### Existing Solutions
<!-- What alternatives exist today, and why do they fall short? -->
| Name | Pros | Cons |
| ---: | :--- | :--- |
| Online genealogy platforms | easy to use, collaborative, connect to records and other families' trees | data lives on someone else's server, often behind a subscription; it is gone or locked once the service ends |
| Desktop genealogy software | mature, feature rich, the data stays local | depends on a large stack of libraries and a specific operating system; unlikely to build or run decades from now without maintenance |
| GEDCOM files | widely supported plain text exchange format | only a format: it still needs a program to browse it; recorded stories and pictures are only referenced, not part of a browsing experience |
| Paper family trees and photo albums | survive for a very long time without any technology | cannot hold audio, cannot be searched, hard to extend and to copy |
<!-- TEMPLATE
| Name | Pros | Cons |
| ---: | :--- | :--- |
| NAME | what it does well | where it falls short |
-->

## Target Users
<!-- Who is this for? One row per user group. -->
| User Group | Description | Needs |
| ---: | :--- | :--- |
| Data collector | a family member who gathers the information about relatives, records stories and collects pictures; not a programmer, and may only speak German | enter, correct and extend the data of a person easily, without breaking the file format |
| Family member | anyone in the family who wants to explore its history, today or generations from now | find people, see how they are related, look at their pictures and listen to their stories |
| Integrator | a programmer who builds their own application on top of the library, e.g. a different user interface | a small, documented API in plain C, and the ability to replace file access, image display and audio playback with their own |
| Future maintainer | a programmer who picks the project up again after it has been shelved for a long time, possibly with only parts of it surviving | build the project with nothing but a C compiler, understand why it is built the way it is, and adapt it to whatever platform exists then |
<!-- TEMPLATE
| User Group | Description | Needs |
| ---: | :--- | :--- |
| NAME | who they are | what they want to achieve with the project |
-->

## Goals
<!-- What the project aims to achieve, split into three tiers following the MoSCoW method. Keep each tier short and concrete. -->

### Must Have (MVP)
<!-- The minimum the project needs to be usable at all. Without these, there is no release. -->
- Read the data of all persons from a folder that holds one subfolder per person, each with a JSON file, and read the relations between persons from a CSV file.
- Show every detail of a person: titles, first and last names, gender, birth and death, professions, places of residence with their dates, and remarks.
- Show the pictures of a person and play their audio recording.
- Search for persons by their details.
- List the relations of a person, and derive their parents, children, siblings, partners and their generation relative to another person.
- Show a family tree around a chosen person, up to a chosen number of steps from family to family, always with whole families.
- Enter the data of a new person in a form and save it as a JSON file in the expected format.
- Run on Windows and Linux.
- The applications are in German.
- The library is written in C99 and builds without any external code.

### Should Have (Target)
<!-- What a realistic, complete version of the project includes. This is what we plan for. -->
- Open an existing person file in the data entry tool to correct or extend it.
- Choose where the data entry tool saves a person file.
- Play the recordings inside the browsing application, with pause and continue, so that family members go through the family tree by listening to the stories of their ancestors.
- Show the pictures of a person inside the browsing application in a carousel, in which the user switches from one picture to the next.
- The text of the applications can be translated into other languages.
- Every data file carries a version, so that future changes to the format can stay compatible with existing data.
- All text is handled as UTF-8, so that names and places in any language are kept intact.
- Applications can replace how the library reads files, shows pictures and plays audio, and fall back to the library's defaults where they do not.
- Every public function of the library is documented.
- A trace of what the library did can be recorded, to find the cause of a problem a user reports.

### Could Have (Stretch)
<!-- What the project would include in a perfect world, if time and resources allow. -->
- Combine several data sources into one family, e.g. sensitive details kept in a separate folder that is not shared openly, or two separately collected trees.
- Enter and edit relations in a tool instead of writing the CSV file by hand.
- The data entry tool creates the person's folder and copies the pictures and the recording into it.
- Run on macOS.
- A tool that migrates data files from an older format version to the current one.
- Resolve conflicting relations, e.g. a person who is their own ancestor, instead of refusing to open the data set.

## Non-Goals
<!-- What the project deliberately does NOT do, not even as a stretch goal. Prevents scope creep. -->
- No online service, account, synchronisation or network access of any kind.
- No external dependencies in the library, not even through submodules or package managers.
- The library is not thread safe; applications that need concurrency take care of it themselves.
- No interpretation or validation of dates, places or gender: they are kept as the free text the data collector entered.
- No compatibility with data files from before the format carried a version.

## Success Criteria
<!-- How do we know we got there? Measurable where possible, ideally one or more per Must Have and Should Have goal. -->
- The library builds from a fresh copy of the repository with only a C99 compiler, without network access.
- The example data set of 18 persons loads, and every field, relation, picture and audio recording that the library reads without a warning can be reached from the browsing application.
- The tree view shows every family within the chosen number of steps from the selected person, with all its parents and children, ordered by generation.
- A data collector who is not a programmer enters a complete person with the data entry tool, and the library reads the saved file without errors.
- A person file saved by the data entry tool can be reopened, changed and saved again without losing any field.
- The whole test suite passes on Windows and Linux, and the memory sanitizer reports no leaks.
- Every public function in the library's headers has documentation.

## Risks and Assumptions
<!-- The few big things that could make the vision fail, and what we take for granted without having verified it. Keep it high-level; detailed questions belong in the open questions document. -->

### Risks
| Risk | Impact | Mitigation |
| ---: | :--- | :--- |
| the data format has to change after data has been collected | existing data can no longer be read, or has to be fixed by hand | version identifier in every data file; keep the format small; change it only with a migration path |
| the operating system interfaces for files, pictures and audio change or disappear | the library builds but can no longer read data or show media | all operating system calls behind one replaceable platform interface |
| the applications' dependencies, e.g. the user interface library, are no longer available | the applications can no longer be built | keep the library free of dependencies, so that a new application can be written on top of it; keep the applications thin |
| memory errors in the C code | crashes or corrupted results that are hard to trace | tests run with sanitizers; tracing of the library's flow |
| the project is shelved and the knowledge about it is lost | nobody can get it running again | records of decisions and their reasons; documented API; plain, human readable data files |
| the rewrite stalls with two incomplete versions side by side | the working version is no longer maintained and the new one is never finished | finish the rewrite package by package, driven by tests, while the original version stays usable |
<!-- TEMPLATE
| Risk | Impact | Mitigation |
| ---: | :--- | :--- |
| what could go wrong | what happens if it does | how we prevent it or notice it early |
-->

### Assumptions
| Assumption | How to Verify |
| ---: | :--- |
| C99 compilers will exist for decades, and C code can be ported by hand if they do not | watch for C compilers on new platforms; archived compilers, e.g. in the GitHub Arctic Code Vault |
| JSON and CSV stay readable to people and programs for decades | they are plain text; check that the files can be read in a text editor |
| a family fits into memory as a whole, so the library can load all data at once; the real data set has passed 100 persons and keeps growing | load a data set with several thousand persons and measure time and memory |
| the data collector keeps every person in its own folder and writes the relations CSV correctly | ask the data collector; check their real data with the library |
| the ids of persons are unique within a data set | the library reports duplicate ids when loading |
| the binary of the library can be used even if its source code is lost | build an application against the binary and the headers only |
<!-- TEMPLATE
| Assumption | How to Verify |
| ---: | :--- |
| what we believe to be true | how we could find out if it is |
-->
