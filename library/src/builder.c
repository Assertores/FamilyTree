// Builder: creating a Context through Questions (docs/decisions/adr_0005.md,
// docs/decisions/adr_0016.md). Every call of FT_Continue either succeeds, or leaves the Builder as
// it was before the call (docs/decisions/adr_0019.md).
//
// A call has two parts. First the answer is brought into the Arenas of the Builder: if the
// Application leaves the Question to the default, the Library answers it the way an Application
// would, and puts what it finds straight into the Arenas: the Default Platform lists folders,
// checks and reads files, and the Library's own parsers parse them. Otherwise the answer of the
// Application is copied into the same Arenas. Then the Builder handles the answer, whoever gave
// it, and asks the next Question. The default can do nothing an Application cannot do
// (docs/decisions/adr_0020.md).

#include "internal.h"

#include <assert.h>
#include <family_tree/api.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static const char thePersonFile[] = "data.json";
static const char theRelationsFile[] = "relations.csv";

// What the Builder waits for: the answer to the Question it asked last.
typedef enum {
	BUILDER_LIST_FOLDERS,
	BUILDER_READ_PERSON_FILE,
	BUILDER_PARSE_PERSON_FILE,
	BUILDER_READ_RELATIONS_FILE,
	BUILDER_PARSE_RELATIONS_FILE,
} BuilderStep;

typedef struct {
	BuilderStep step;
	FT_Question question;
	const char* const* folders;
	size_t folderCount;
	size_t folderIndex;
	ContextPerson* persons; // one for each folder at most, in the scratch Arena
	size_t personCount;
	bool warned;		  // during the whole creation
	const char* original; // the unchanged content of the file a parsing Question is about
	size_t length;		  // of the content, without its terminating zero
} BuilderState;

// The Arenas of a creation, each with a lifetime of its own (docs/decisions/adr_0019.md).
struct FT_Builder {
	Arena kept;		// becomes the Arena of the Context
	Arena pending;	// what the file being read adds to the Context if it is accepted
	Arena original; // the unchanged content of the file being read, until it is parsed
	Arena scratch;	// everything else temporary, until the creation is over
	const char* dataFolder;
	BuilderState state;
};

// The marks of all Arenas at the start of a call, to roll a failed call back.
typedef struct {
	ArenaMark kept;
	ArenaMark pending;
	ArenaMark original;
	ArenaMark scratch;
} Marks;

// What a successful call does with the pending Arena.
typedef enum {
	PENDING_OPEN, // the file is still being read
	PENDING_KEEP, // moved into the Context Arena, as the file was accepted
	PENDING_DROP, // freed, as the file was skipped
} PendingOutcome;

// One call of FT_Continue. It works on a copy of the state, which replaces the state of the
// Builder only if the call succeeds.
typedef struct {
	FT_Builder* builder;
	BuilderState next;
	Arena* scratch;
	FT_Trace* trace;
	FT_Context* context; // set when the creation is over
	PendingOutcome pending;
	FT_PersonRecord person;	   // the parsed Person File of the answer, in the Arenas of the Builder
	FT_RelationList relations; // the parsed Relations File of the answer, likewise
} Call;

static bool
PrivIsSuccess(FT_Result aResult) {
	return aResult == FT_OK || aResult == FT_OK_WITH_WARNINGS;
}

FT_Result
PublicResult(Result aResult) {
	switch (aResult) {
	case RESULT_OK:
		return FT_OK;
	case RESULT_OK_WITH_WARNINGS:
		return FT_OK_WITH_WARNINGS;
	case RESULT_OUT_OF_MEMORY:
		return FT_ERROR_OUT_OF_MEMORY;
	case RESULT_UNREADABLE:
		break; // the caller works around it, e.g. by skipping the file
	}
	assert(false && "every failure without a public counterpart is handled by the caller");
	return FT_ERROR_UNEXPECTED_ANSWER; // ends the call, which rolls the answer back
}

// The result of two steps of one call: the first error, otherwise any warning.
static FT_Result
PrivCombine(FT_Result aFirst, FT_Result aSecond) {
	if (!PrivIsSuccess(aFirst)) {
		return aFirst;
	}
	if (aSecond != FT_OK) {
		return aSecond;
	}
	return aFirst;
}

static FT_Result
PrivUnexpected(const Call* aCall, const char* aReason) {
	TraceError(aCall->trace, "unexpected answer: %s", aReason);
	return FT_ERROR_UNEXPECTED_ANSWER;
}

static void
PrivFreeBuilder(FT_Builder* aBuilder) {
	ArenaFree(&aBuilder->kept);
	ArenaFree(&aBuilder->pending);
	ArenaFree(&aBuilder->original);
	ArenaFree(&aBuilder->scratch);
	free(aBuilder);
}

static const char*
PrivQuestionName(FT_QuestionType aType) {
	switch (aType) {
	case FT_QUESTION_LIST_FOLDERS:
		return "LIST_FOLDERS";
	case FT_QUESTION_FILE_EXISTS:
		return "FILE_EXISTS";
	case FT_QUESTION_READ_FILE:
		return "READ_FILE";
	case FT_QUESTION_PARSE_PERSON_FILE:
		return "PARSE_PERSON_FILE";
	case FT_QUESTION_PARSE_RELATIONS_FILE:
		return "PARSE_RELATIONS_FILE";
	case FT_QUESTION_NONE:
		return "NONE";
	}
	return "an unknown Question";
}

// A path of a Question relative to the Data Folder, as warnings name a file. An Application can
// make it the same way, as it knows the Data Folder it passed.
static const char*
PrivRelativePath(const FT_Builder* aBuilder, const char* aPath) {
	if (!StartsWith(aPath, aBuilder->dataFolder)) {
		return aPath;
	}
	const char* rest = aPath + strlen(aBuilder->dataFolder);
	return rest[0] == '/' ? rest + 1 : rest;
}

// The file the open Question is about, relative to the Data Folder.
static const char*
PrivFile(const Call* aCall) {
	return PrivRelativePath(aCall->builder, aCall->next.question.path);
}

// ----- The default answers

// Parses the Person File in place with the Library's own parser, and answers with what it read; a
// file it cannot read is answered without a record, as an Application would. The arrays of the
// Person go into the pending Arena, like the copy of an Application's answer.
static FT_Result
PrivParsePersonByDefault(Call* aCall, FT_Answer* aOut) {
	const Result result = ParsePersonFile(
		aCall->next.question.text,
		PrivFile(aCall),
		aCall->scratch,
		&aCall->builder->pending,
		&aCall->person,
		aCall->trace);
	if (result == RESULT_UNREADABLE) {
		TraceStep(aCall->trace, "answering without a record");
		return FT_OK; // skipped with a warning, like every answer without a record
	}
	if (result == RESULT_OUT_OF_MEMORY) {
		return PublicResult(result);
	}
	aOut->person = &aCall->person;
	return PublicResult(result);
}

// Parses the Relations File like PrivParsePersonByDefault. The Relations go into the scratch
// Arena, like the copy of an Application's answer, as the Context copies each Relation to its
// Persons.
static FT_Result
PrivParseRelationsByDefault(Call* aCall, FT_Answer* aOut) {
	const Result result = ParseRelationsFile(
		aCall->next.question.text,
		PrivFile(aCall),
		aCall->scratch,
		aCall->scratch,
		&aCall->relations,
		aCall->trace);
	if (result == RESULT_UNREADABLE) {
		TraceStep(aCall->trace, "answering without Relations");
		return FT_OK; // skipped with a warning, like every answer without Relations
	}
	if (result == RESULT_OUT_OF_MEMORY) {
		return PublicResult(result);
	}
	aOut->relations = &aCall->relations;
	return PublicResult(result);
}

// Answers a Question the Application left to the default, the way an Application would: the
// Default Platform lists folders, checks and reads files, and the Library's own parsers parse
// them. Each puts its result straight into the Arena where PrivCopyAnswer puts the answer of an
// Application, so nothing has to be copied.
static FT_Result
PrivAnswerByDefault(Call* aCall, FT_Answer* aOut) {
	const FT_Question* question = &aCall->next.question;
	char* content = NULL;
	FT_Result result = FT_OK;
	switch (question->type) {
	case FT_QUESTION_LIST_FOLDERS:
		return PlatformListFolders(
			question->path,
			&aCall->builder->kept,
			&aOut->folderNames,
			&aOut->folderCount,
			aCall->trace);
	case FT_QUESTION_FILE_EXISTS:
		return PlatformFileExists(question->path, &aOut->exists, aCall->trace);
	case FT_QUESTION_READ_FILE:
		result = PlatformReadFile(
			question->path,
			&aCall->builder->original,
			&content,
			&aOut->found,
			aCall->trace);
		aOut->content = content;
		return result;
	case FT_QUESTION_PARSE_PERSON_FILE:
		return PrivParsePersonByDefault(aCall, aOut);
	case FT_QUESTION_PARSE_RELATIONS_FILE:
		return PrivParseRelationsByDefault(aCall, aOut);
	case FT_QUESTION_NONE:
		break;
	}
	return PrivUnexpected(aCall, "no Question is open");
}

// ----- Copying the answer of an Application

// Whether a text lies within the text of the parsing Question, with its terminating zero.
// Addresses are compared as integers, as comparing pointers into different objects is undefined in
// C; every platform with flat memory supports it (docs/decisions/adr_0020.md).
static bool
PrivLiesWithin(const Call* aCall, const char* aText) {
	if (aCall->next.question.text == NULL) {
		return false;
	}
	const uintptr_t start = (uintptr_t)aCall->next.question.text;
	const uintptr_t end = start + aCall->next.length;
	const uintptr_t text = (uintptr_t)aText;
	if (text < start || text > end) {
		return false;
	}
	return memchr(aText, '\0', (size_t)(end - text) + 1) != NULL;
}

// Copies a text into the pending Arena, unless it is missing or lies within the text of the
// parsing Question. Returns false if memory ran out.
static bool
PrivCopyText(const Call* aCall, const char** aText) {
	if (*aText == NULL || PrivLiesWithin(aCall, *aText)) {
		return true;
	}
	*aText = ArenaCopyText(&aCall->builder->pending, *aText);
	return *aText != NULL;
}

// Copies several texts like PrivCopyText. Returns false if memory ran out.
static bool
PrivCopyEachText(const Call* aCall, const char** const* aTexts, size_t aCount) {
	for (size_t i = 0; i < aCount; ++i) {
		if (!PrivCopyText(aCall, aTexts[i])) {
			return false;
		}
	}
	return true;
}

// The items of an empty list, which only tell it apart from a missing one and are never read.
static const char* const theNoTexts[1] = {NULL};
static const FT_Residence theNoResidences[1] = {{NULL, NULL, NULL}};

// Copies an array of texts, and the texts, into the pending Arena. Returns false if memory ran
// out.
static bool
PrivCopyTexts(const Call* aCall, const char* const** aItems, size_t aCount) {
	if (*aItems == NULL) {
		return true;
	}
	if (aCount == 0) {
		*aItems = theNoTexts;
		return true;
	}
	if (aCount > SIZE_MAX / sizeof(const char*)) {
		return false;
	}
	const char** items =
		(const char**)ArenaAllocate(&aCall->builder->pending, aCount * sizeof(const char*));
	if (items == NULL) {
		return false;
	}
	for (size_t i = 0; i < aCount; ++i) {
		items[i] = (*aItems)[i];
		if (!PrivCopyText(aCall, &items[i])) {
			return false;
		}
	}
	*aItems = items;
	return true;
}

static bool
PrivCopyTextList(const Call* aCall, FT_TextList* aList) {
	return PrivCopyTexts(aCall, &aList->items, aList->count);
}

// Copies the Places of Residence like PrivCopyTexts.
static bool
PrivCopyResidences(const Call* aCall, FT_ResidenceList* aList) {
	if (aList->items == NULL) {
		return true;
	}
	if (aList->count == 0) {
		aList->items = theNoResidences;
		return true;
	}
	if (aList->count > SIZE_MAX / sizeof(FT_Residence)) {
		return false;
	}
	FT_Residence* items =
		ArenaAllocate(&aCall->builder->pending, aList->count * sizeof(FT_Residence));
	if (items == NULL) {
		return false;
	}
	for (size_t i = 0; i < aList->count; ++i) {
		items[i] = aList->items[i];
		const char** texts[] = {&items[i].name, &items[i].startDate, &items[i].endDate};
		if (!PrivCopyEachText(aCall, texts, sizeof(texts) / sizeof(texts[0]))) {
			return false;
		}
	}
	aList->items = items;
	return true;
}

static bool
PrivCopyDetails(const Call* aCall, FT_Person* aPerson) {
	const char** texts[] = {
		&aPerson->title,
		&aPerson->titleOfNobility,
		&aPerson->gender,
		&aPerson->dateOfBirth,
		&aPerson->placeOfBirth,
		&aPerson->dateOfDeath,
		&aPerson->placeOfDeath,
	};
	if (!PrivCopyEachText(aCall, texts, sizeof(texts) / sizeof(texts[0]))) {
		return false;
	}
	FT_TextList* lists[] = {
		&aPerson->firstNames,
		&aPerson->lastNames,
		&aPerson->professions,
		&aPerson->remarks,
	};
	for (size_t i = 0; i < sizeof(lists) / sizeof(lists[0]); ++i) {
		if (!PrivCopyTextList(aCall, lists[i])) {
			return false;
		}
	}
	return PrivCopyResidences(aCall, &aPerson->placesOfResidence);
}

// Copies a parsed Person File into aCall->person, with its Media paths.
static FT_Result
PrivCopyPerson(Call* aCall, const FT_Answer* aAnswer, FT_Answer* aOut) {
	if (aAnswer->person == NULL) {
		return FT_OK;
	}
	FT_PersonRecord* record = &aCall->person;
	*record = *aAnswer->person;
	if (!PrivCopyDetails(aCall, &record->person)) {
		return FT_ERROR_OUT_OF_MEMORY;
	}
	if (!PrivCopyText(aCall, &record->recording)) {
		return FT_ERROR_OUT_OF_MEMORY;
	}
	if (!PrivCopyTexts(aCall, &record->pictures, record->pictureCount)) {
		return FT_ERROR_OUT_OF_MEMORY;
	}
	aOut->person = record;
	return FT_OK;
}

// Copies a parsed Relations File into aCall->relations: the array into the scratch Arena, as the
// Context copies each Relation to its Persons, and their texts into the pending Arena. Relations
// that are NULL stay NULL, for the handler to judge.
static FT_Result
PrivCopyRelations(Call* aCall, const FT_Answer* aAnswer, FT_Answer* aOut) {
	if (aAnswer->relations == NULL) {
		return FT_OK;
	}
	const FT_RelationList* from = aAnswer->relations;
	const size_t count = from->count;
	aCall->relations.items = NULL;
	aCall->relations.count = count;
	aOut->relations = &aCall->relations;
	if (from->items == NULL || count == 0) {
		return FT_OK;
	}
	if (count > SIZE_MAX / sizeof(FT_Relation)) {
		return FT_ERROR_OUT_OF_MEMORY;
	}
	FT_Relation* relations = ArenaAllocate(aCall->scratch, count * sizeof(FT_Relation));
	if (relations == NULL) {
		return FT_ERROR_OUT_OF_MEMORY;
	}
	for (size_t i = 0; i < count; ++i) {
		relations[i] = from->items[i];
		const char** texts[] = {
			&relations[i].relationship,
			&relations[i].startDate,
			&relations[i].endDate,
		};
		if (!PrivCopyEachText(aCall, texts, sizeof(texts) / sizeof(texts[0]))) {
			return FT_ERROR_OUT_OF_MEMORY;
		}
	}
	aCall->relations.items = relations;
	return FT_OK;
}

// Copies the folder names into the Context Arena, which keeps the names of the Person Folders.
// They are checked here, as they are copied one by one. Without folders, the names are NULL, as
// the Default Platform gives them.
static FT_Result
PrivCopyFolderNames(const Call* aCall, const FT_Answer* aAnswer, FT_Answer* aOut) {
	const size_t count = aAnswer->folderCount;
	if (count == 0) {
		return FT_OK;
	}
	if (aAnswer->folderNames == NULL) {
		return PrivUnexpected(aCall, "the folder names are NULL");
	}
	if (count > SIZE_MAX / sizeof(const char*)) {
		return FT_ERROR_OUT_OF_MEMORY;
	}
	Arena* kept = &aCall->builder->kept;
	const char** names = (const char**)ArenaAllocate(kept, count * sizeof(const char*));
	if (names == NULL) {
		return FT_ERROR_OUT_OF_MEMORY;
	}
	for (size_t i = 0; i < count; ++i) {
		if (aAnswer->folderNames[i] == NULL) {
			return PrivUnexpected(aCall, "a folder name is NULL");
		}
		names[i] = ArenaCopyText(kept, aAnswer->folderNames[i]);
		if (names[i] == NULL) {
			return FT_ERROR_OUT_OF_MEMORY;
		}
	}
	aOut->folderNames = names;
	aOut->folderCount = count;
	return FT_OK;
}

// Copies the content of a file into the original Arena, where it waits unchanged until the file
// is parsed. A file that was not found has no content, as the Default Platform gives it.
static FT_Result
PrivCopyContent(const Call* aCall, const FT_Answer* aAnswer, FT_Answer* aOut) {
	aOut->found = aAnswer->found;
	if (!aAnswer->found || aAnswer->content == NULL) {
		return FT_OK;
	}
	aOut->content = ArenaCopyText(&aCall->builder->original, aAnswer->content);
	return aOut->content != NULL ? FT_OK : FT_ERROR_OUT_OF_MEMORY;
}

// Copies an Application's answer into the zeroed aOut, so that it looks like an answer of the
// default: it holds only the fields of the open Question, and each of its pointers is NULL or
// points into the Arenas of the Builder, where the default puts its own answer. The folder names
// go into the Context Arena, the content of a file into the original Arena, and a parsed record
// into the pending Arena, except for its texts that lie within the text of the parsing Question
// (docs/decisions/adr_0020.md). Apart from the folder names, what is NULL stays NULL, for the
// handlers to judge.
static FT_Result
PrivCopyAnswer(Call* aCall, const FT_Answer* aAnswer, FT_Answer* aOut) {
	switch (aCall->next.question.type) {
	case FT_QUESTION_LIST_FOLDERS:
		return PrivCopyFolderNames(aCall, aAnswer, aOut);
	case FT_QUESTION_FILE_EXISTS:
		aOut->exists = aAnswer->exists;
		return FT_OK;
	case FT_QUESTION_READ_FILE:
		return PrivCopyContent(aCall, aAnswer, aOut);
	case FT_QUESTION_PARSE_PERSON_FILE:
		return PrivCopyPerson(aCall, aAnswer, aOut);
	case FT_QUESTION_PARSE_RELATIONS_FILE:
		return PrivCopyRelations(aCall, aAnswer, aOut);
	case FT_QUESTION_NONE:
		break;
	}
	return PrivUnexpected(aCall, "no Question is open");
}

// ----- Asking

// Asks for the Person File of the next folder, or for the Relations File after the last folder.
static FT_Result
PrivAskNext(Call* aCall) {
	BuilderState* next = &aCall->next;
	const char* path = NULL;
	if (next->folderIndex < next->folderCount) {
		path = JoinPath(
			aCall->scratch,
			aCall->builder->dataFolder,
			next->folders[next->folderIndex],
			thePersonFile);
		next->step = BUILDER_READ_PERSON_FILE;
	} else {
		path = JoinPath(aCall->scratch, aCall->builder->dataFolder, theRelationsFile, NULL);
		next->step = BUILDER_READ_RELATIONS_FILE;
	}
	if (path == NULL) {
		return FT_ERROR_OUT_OF_MEMORY;
	}
	next->question.type = FT_QUESTION_READ_FILE;
	next->question.path = path;
	next->question.text = NULL;
	return FT_OK;
}

// Asks to parse the content of a file that was read, which waits unchanged in the original Arena.
// The text of the Question is a copy of it in the pending Arena, which may be changed while it is
// parsed, and stays in the Context; the path of the Question stays that of the file
// (docs/decisions/adr_0020.md).
static FT_Result
PrivAskToParse(Call* aCall, const FT_Answer* aAnswer, FT_QuestionType aType, BuilderStep aStep) {
	if (aAnswer->content == NULL) {
		return PrivUnexpected(aCall, "the file was found, but its content is NULL");
	}
	char* text = ArenaCopyText(&aCall->builder->pending, aAnswer->content);
	if (text == NULL) {
		return FT_ERROR_OUT_OF_MEMORY;
	}
	aCall->next.step = aStep;
	aCall->next.question.type = aType;
	aCall->next.question.text = text;
	aCall->next.original = aAnswer->content;
	aCall->next.length = strlen(aAnswer->content);
	return FT_OK;
}

// ----- Handling the answers, which lie in the Arenas of the Builder

static FT_Result
PrivHandleListFolders(Call* aCall, const FT_Answer* aAnswer) {
	const size_t count = aAnswer->folderCount;
	TraceStep(aCall->trace, "keeping %zu folders of the Data Folder", count);
	if (count >= SIZE_MAX / sizeof(ContextPerson)) {
		return FT_ERROR_OUT_OF_MEMORY;
	}
	// The Context keeps a sorted copy of the Persons, so the Builder's own array is temporary.
	ContextPerson* persons = ArenaAllocate(aCall->scratch, (count + 1) * sizeof(ContextPerson));
	if (persons == NULL) {
		return FT_ERROR_OUT_OF_MEMORY;
	}
	aCall->next.folders = aAnswer->folderNames;
	aCall->next.folderCount = count;
	aCall->next.persons = persons;
	return PrivAskNext(aCall);
}

static FT_Result
PrivHandleReadPersonFile(Call* aCall, const FT_Answer* aAnswer) {
	if (!aAnswer->found) {
		TraceWarning(
			aCall->trace,
			"%s: no Person File %s, skipped",
			aCall->next.folders[aCall->next.folderIndex],
			thePersonFile);
		++aCall->next.folderIndex;
		return PrivCombine(FT_OK_WITH_WARNINGS, PrivAskNext(aCall));
	}
	return PrivAskToParse(aCall, aAnswer, FT_QUESTION_PARSE_PERSON_FILE, BUILDER_PARSE_PERSON_FILE);
}

// Adds a parsed Person; the file is kept for the Person, or dropped if it is skipped. Whoever
// parsed the file checked its Format Version (docs/decisions/adr_0020.md).
static FT_Result
PrivAcceptPerson(Call* aCall, const FT_PersonRecord* aRecord, const char* aFile) {
	aCall->pending = PENDING_DROP;
	if (aRecord == NULL) {
		TraceWarning(aCall->trace, "%s: skipped, it could not be read", aFile);
		return FT_OK_WITH_WARNINGS;
	}
	TraceStep(aCall->trace, "%s: taking over Person Id %lld", aFile, aRecord->person.id);
	BuilderState* next = &aCall->next;
	// The Relations of each Person are collected when the Context is created.
	const ContextPerson person = {
		CompletePerson(&aRecord->person),
		next->folders[next->folderIndex],
		NULL,
		0,
	};
	next->persons[next->personCount] = person;
	aCall->pending = PENDING_KEEP;
	++next->personCount;
	return FT_OK;
}

static FT_Result
PrivHandleParsePersonFile(Call* aCall, const FT_Answer* aAnswer) {
	const FT_Result result = PrivAcceptPerson(aCall, aAnswer->person, PrivFile(aCall));
	if (!PrivIsSuccess(result)) {
		return result;
	}
	++aCall->next.folderIndex;
	return PrivCombine(result, PrivAskNext(aCall));
}

// Creates the Context from everything read; the creation is over afterwards.
static FT_Result
PrivFinish(Call* aCall, const FT_Relation* aRelations, size_t aRelationCount) {
	return CreateContext(
		&aCall->builder->kept,
		aCall->scratch,
		aCall->next.persons,
		aCall->next.personCount,
		aRelations,
		aRelationCount,
		&aCall->context,
		aCall->trace);
}

static FT_Result
PrivHandleReadRelationsFile(Call* aCall, const FT_Answer* aAnswer) {
	if (!aAnswer->found) {
		TraceWarning(aCall->trace, "%s: missing, the Data Set has no Relations", PrivFile(aCall));
		return PrivCombine(FT_OK_WITH_WARNINGS, PrivFinish(aCall, NULL, 0));
	}
	return PrivAskToParse(
		aCall,
		aAnswer,
		FT_QUESTION_PARSE_RELATIONS_FILE,
		BUILDER_PARSE_RELATIONS_FILE);
}

static FT_Result
PrivCheckRelations(const Call* aCall, const FT_RelationList* aList) {
	if (aList->count > 0 && aList->items == NULL) {
		return PrivUnexpected(aCall, "the Relations are NULL");
	}
	for (size_t i = 0; i < aList->count; ++i) {
		const FT_RelationType type = aList->items[i].type;
		if (type != FT_RELATION_UNRESTRICTED && type != FT_RELATION_STRICTLY_LOWER
			&& type != FT_RELATION_STRICTLY_HIGHER) {
			return PrivUnexpected(aCall, "a Relation has an unknown Relation Type");
		}
	}
	return FT_OK;
}

// Creates the Context with the Relations, or without them if the Relations File is skipped.
// Whoever parsed the file checked its Format Version (docs/decisions/adr_0020.md).
static FT_Result
PrivHandleParseRelationsFile(Call* aCall, const FT_Answer* aAnswer) {
	const FT_RelationList* list = aAnswer->relations;
	if (list == NULL) {
		TraceWarning(aCall->trace, "%s: skipped, it could not be read", PrivFile(aCall));
		aCall->pending = PENDING_DROP;
		return PrivCombine(FT_OK_WITH_WARNINGS, PrivFinish(aCall, NULL, 0));
	}
	TraceStep(aCall->trace, "%s: taking over %zu Relations", PrivFile(aCall), list->count);
	if (PrivCheckRelations(aCall, list) != FT_OK) {
		return FT_ERROR_UNEXPECTED_ANSWER;
	}
	aCall->pending = PENDING_KEEP;
	return PrivFinish(aCall, list->items, list->count);
}

// Handles the answer to the open Question, whoever gave it, and asks the next Question.
static FT_Result
PrivHandleAnswer(Call* aCall, const FT_Answer* aAnswer) {
	switch (aCall->next.step) {
	case BUILDER_LIST_FOLDERS:
		return PrivHandleListFolders(aCall, aAnswer);
	case BUILDER_READ_PERSON_FILE:
		return PrivHandleReadPersonFile(aCall, aAnswer);
	case BUILDER_PARSE_PERSON_FILE:
		return PrivHandleParsePersonFile(aCall, aAnswer);
	case BUILDER_READ_RELATIONS_FILE:
		return PrivHandleReadRelationsFile(aCall, aAnswer);
	case BUILDER_PARSE_RELATIONS_FILE:
		return PrivHandleParseRelationsFile(aCall, aAnswer);
	}
	return PrivUnexpected(aCall, "the Builder waits for no known answer");
}

// ----- Ending a call

static Marks
PrivMarks(const FT_Builder* aBuilder) {
	const Marks marks = {
		ArenaGetMark(&aBuilder->kept),
		ArenaGetMark(&aBuilder->pending),
		ArenaGetMark(&aBuilder->original),
		ArenaGetMark(&aBuilder->scratch),
	};
	return marks;
}

// Undoes a failed call: frees what it allocated, and restores the text of a parsing Question,
// which the Library or the Application may have changed (docs/decisions/adr_0020.md).
static void
PrivRollBack(FT_Builder* aBuilder, const Marks* aMarks) {
	ArenaReset(&aBuilder->kept, aMarks->kept);
	ArenaReset(&aBuilder->pending, aMarks->pending);
	ArenaReset(&aBuilder->original, aMarks->original);
	ArenaReset(&aBuilder->scratch, aMarks->scratch);
	const BuilderState* state = &aBuilder->state;
	if (state->question.type == FT_QUESTION_PARSE_PERSON_FILE
		|| state->question.type == FT_QUESTION_PARSE_RELATIONS_FILE) {
		(void)memcpy(state->question.text, state->original, state->length + 1);
	}
}

// Ends a successful call: either the creation is over, or the next Question is open.
static FT_Result
PrivCommit(Call* aCall, FT_Result aResult, FT_Question* aOutQuestion, FT_Context** aOutContext) {
	FT_Builder* builder = aCall->builder;
	aCall->next.warned = aCall->next.warned || aResult == FT_OK_WITH_WARNINGS;
	if (aCall->pending == PENDING_KEEP) {
		ArenaMoveAll(&builder->pending, &builder->kept);
	} else if (aCall->pending == PENDING_DROP) {
		ArenaFree(&builder->pending);
	}
	if (aCall->pending != PENDING_OPEN) {
		// The file is done, so its parsing Question will not be answered again.
		ArenaFree(&builder->original);
	}
	if (aCall->context != NULL) {
		// The Context takes over the Arena that holds it.
		aCall->context->arena = builder->kept;
		builder->kept.last = NULL;
		*aOutContext = aCall->context;
		(void)memset(aOutQuestion, 0, sizeof(*aOutQuestion));
		const bool warned = aCall->next.warned;
		PrivFreeBuilder(builder);
		return warned ? FT_OK_WITH_WARNINGS : FT_OK;
	}
	builder->state = aCall->next;
	*aOutQuestion = builder->state.question;
	return aResult;
}

// ----- Public functions

FT_Result
FT_Create(const char* aDataFolder, FT_Context** aOutContext, FT_Trace* aOutTrace) {
	// Stub until the work package Media and Default Platform.
	TraceOpen(aOutTrace, "FT_Create", "%s", aDataFolder != NULL ? aDataFolder : "");
	(void)aOutContext;
	TraceError(aOutTrace, "not supported yet");
	return TraceEndCall(aOutTrace, FT_ERROR_NOT_SUPPORTED);
}

FT_Result
FT_StartCreate(
	const char* aDataFolder,
	FT_Builder** aOutBuilder,
	FT_Question* aOutQuestion,
	FT_Trace* aOutTrace) {
	TraceOpen(aOutTrace, "FT_StartCreate", "%s", aDataFolder != NULL ? aDataFolder : "");
	if (aDataFolder == NULL || aOutBuilder == NULL || aOutQuestion == NULL) {
		TraceError(aOutTrace, "the Data Folder, the Builder or the Question is NULL");
		return TraceEndCall(aOutTrace, FT_ERROR_INVALID_ARGUMENT);
	}
	FT_Builder* builder = calloc(1, sizeof(FT_Builder));
	if (builder == NULL) {
		return TraceEndCall(aOutTrace, FT_ERROR_OUT_OF_MEMORY);
	}
	builder->dataFolder = ArenaCopyText(&builder->kept, aDataFolder);
	if (builder->dataFolder == NULL) {
		PrivFreeBuilder(builder);
		return TraceEndCall(aOutTrace, FT_ERROR_OUT_OF_MEMORY);
	}
	builder->state.step = BUILDER_LIST_FOLDERS;
	builder->state.question.type = FT_QUESTION_LIST_FOLDERS;
	builder->state.question.path = builder->dataFolder;
	TraceStep(aOutTrace, "asking for the folders of the Data Folder");
	*aOutBuilder = builder;
	*aOutQuestion = builder->state.question;
	return TraceEndCall(aOutTrace, FT_OK);
}

FT_Result
FT_Continue(
	FT_Builder* aBuilder,
	const FT_Answer* aAnswer,
	FT_Question* aOutQuestion,
	FT_Context** aOutContext,
	FT_Trace* aOutTrace) {
	TraceOpen(
		aOutTrace,
		"FT_Continue",
		"answer to %s %s",
		aBuilder != NULL ? PrivQuestionName(aBuilder->state.question.type) : "",
		aBuilder != NULL ? aBuilder->state.question.path : "");
	if (aBuilder == NULL || aAnswer == NULL || aOutQuestion == NULL || aOutContext == NULL) {
		TraceError(aOutTrace, "the Builder, the answer, the Question or the Context is NULL");
		return TraceEndCall(aOutTrace, FT_ERROR_INVALID_ARGUMENT);
	}
	Call call;
	(void)memset(&call, 0, sizeof(call));
	call.builder = aBuilder;
	call.next = aBuilder->state;
	call.scratch = &aBuilder->scratch;
	call.trace = aOutTrace;
	const Marks marks = PrivMarks(aBuilder);
	FT_Answer answer;
	(void)memset(&answer, 0, sizeof(answer));
	FT_Result result = FT_OK;
	if (aAnswer->useDefault) {
		TraceStep(aOutTrace, "answering by default");
		result = PrivAnswerByDefault(&call, &answer);
	} else {
		TraceStep(aOutTrace, "copying the answer of the Application");
		result = PrivCopyAnswer(&call, aAnswer, &answer);
	}
	if (PrivIsSuccess(result)) {
		result = PrivCombine(result, PrivHandleAnswer(&call, &answer));
	}
	if (!PrivIsSuccess(result)) {
		PrivRollBack(aBuilder, &marks);
		TraceError(aOutTrace, "nothing of this answer was kept; answer again, or cancel");
		return TraceEndCall(aOutTrace, result);
	}
	return TraceEndCall(aOutTrace, PrivCommit(&call, result, aOutQuestion, aOutContext));
}

FT_Result
FT_CancelCreate(FT_Builder* aBuilder, FT_Trace* aOutTrace) {
	TraceOpen(aOutTrace, "FT_CancelCreate", "");
	if (aBuilder == NULL) {
		TraceError(aOutTrace, "the Builder is NULL");
		return TraceEndCall(aOutTrace, FT_ERROR_INVALID_ARGUMENT);
	}
	PrivFreeBuilder(aBuilder);
	return TraceEndCall(aOutTrace, FT_OK);
}
