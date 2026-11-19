// Context: built once from the parsed data, and freed as a whole (docs/decisions/adr_0003.md,
// docs/decisions/adr_0019.md).

#include "internal.h"

#include <assert.h>
#include <family_tree/api.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static const char theRelationsFile[] = "relations.csv";

// The Default String, which every missing Detail points to, and the empty text, which every
// Detail that is empty in the Person File points to. Two arrays, so their addresses differ
// (docs/decisions/adr_0020.md).
static const char theDefaultString[1] = "";
static const char theEmptyText[1] = "";
static const char* const theMissingList[1] = {theDefaultString};
static const char* const theEmptyList[1] = {theEmptyText};
static const FT_Residence theMissingResidences[1] = {
	{theDefaultString, theDefaultString, theDefaultString},
};
static const FT_Residence theEmptyResidences[1] = {
	{theEmptyText, theEmptyText, theEmptyText},
};

bool
IsDefaultString(const char* aText) {
	return aText == theDefaultString;
}

// ----- Completing parsed data

// A missing text becomes the Default String.
static const char*
PrivComplete(const char* aText) {
	return aText != NULL ? aText : theDefaultString;
}

// Every array of a parsed record lies in the Arenas of the Builder, which allocated it without
// const, also when an Application gave the record (docs/decisions/adr_0019.md); so its missing
// texts are completed where they are. The static arrays of an empty list are never written, as they
// have no items.
static const char**
PrivWritableTexts(const char* const* aItems) {
	return (const char**)(uintptr_t)aItems;
}

static FT_Residence*
PrivWritableResidences(const FT_Residence* aItems) {
	return (FT_Residence*)(uintptr_t)aItems;
}

// A missing list gets the static array of a missing list, an empty one that of an empty list;
// otherwise each missing text in it becomes the Default String.
static void
PrivCompleteList(FT_TextList* aList) {
	if (aList->items == NULL) {
		aList->items = theMissingList;
		aList->count = 0;
		return;
	}
	if (aList->count == 0) {
		aList->items = theEmptyList;
		return;
	}
	const char** items = PrivWritableTexts(aList->items);
	for (size_t i = 0; i < aList->count; ++i) {
		items[i] = PrivComplete(items[i]);
	}
}

// Completes the Places of Residence like PrivCompleteList.
static void
PrivCompleteResidences(FT_ResidenceList* aList) {
	if (aList->items == NULL) {
		aList->items = theMissingResidences;
		aList->count = 0;
		return;
	}
	if (aList->count == 0) {
		aList->items = theEmptyResidences;
		return;
	}
	FT_Residence* items = PrivWritableResidences(aList->items);
	for (size_t i = 0; i < aList->count; ++i) {
		items[i].name = PrivComplete(items[i].name);
		items[i].startDate = PrivComplete(items[i].startDate);
		items[i].endDate = PrivComplete(items[i].endDate);
	}
}

FT_Person
CompletePerson(const FT_Person* aPerson) {
	FT_Person person = *aPerson;
	person.title = PrivComplete(person.title);
	person.titleOfNobility = PrivComplete(person.titleOfNobility);
	person.gender = PrivComplete(person.gender);
	person.dateOfBirth = PrivComplete(person.dateOfBirth);
	person.placeOfBirth = PrivComplete(person.placeOfBirth);
	person.dateOfDeath = PrivComplete(person.dateOfDeath);
	person.placeOfDeath = PrivComplete(person.placeOfDeath);
	PrivCompleteList(&person.firstNames);
	PrivCompleteList(&person.lastNames);
	PrivCompleteList(&person.professions);
	PrivCompleteResidences(&person.placesOfResidence);
	PrivCompleteList(&person.remarks);
	return person;
}

static FT_Relation
PrivCompleteRelation(const FT_Relation* aRelation) {
	FT_Relation relation = *aRelation;
	relation.relationship = PrivComplete(aRelation->relationship);
	relation.startDate = PrivComplete(aRelation->startDate);
	relation.endDate = PrivComplete(aRelation->endDate);
	return relation;
}

// ----- Index of the Persons

// Merges the sorted runs [aLeft, aMiddle) and [aMiddle, aRight) of aFrom into aTo; on equal
// Person Ids the left run comes first, which keeps the sort stable.
static void
PrivMerge(
	const ContextPerson* aFrom,
	ContextPerson* aTo,
	size_t aLeft,
	size_t aMiddle,
	size_t aRight) {
	size_t left = aLeft;
	size_t right = aMiddle;
	for (size_t i = aLeft; i < aRight; ++i) {
		if (left < aMiddle
			&& (right >= aRight || aFrom[left].person.id <= aFrom[right].person.id)) {
			aTo[i] = aFrom[left];
			++left;
		} else {
			aTo[i] = aFrom[right];
			++right;
		}
	}
}

static size_t
PrivMinimum(size_t aFirst, size_t aSecond) {
	return aFirst < aSecond ? aFirst : aSecond;
}

// Sorts by Person Id with a bottom-up merge sort; aWork is as large as aPersons.
static void
PrivSort(ContextPerson* aPersons, ContextPerson* aWork, size_t aCount) {
	ContextPerson* from = aPersons;
	ContextPerson* target = aWork;
	for (size_t width = 1; width < aCount; width *= 2) {
		for (size_t left = 0; left < aCount; left += 2 * width) {
			const size_t middle = PrivMinimum(left + width, aCount);
			PrivMerge(from, target, left, middle, PrivMinimum(middle + width, aCount));
		}
		ContextPerson* swap = from;
		from = target;
		target = swap;
	}
	if (from != aPersons) {
		(void)memcpy(aPersons, from, aCount * sizeof(ContextPerson));
	}
}

// Keeps one Person of each Person Id: the one whose Person Folder comes first in byte order
// (docs/requirements/req_0053.md). Returns the number of Persons kept.
static size_t
PrivRemoveDuplicates(ContextPerson* aPersons, size_t aCount, FT_Trace* aTrace, bool* aWarned) {
	size_t kept = 0;
	for (size_t start = 0, end = 0; start < aCount; start = end) {
		size_t best = start;
		for (end = start + 1; end < aCount && aPersons[end].person.id == aPersons[start].person.id;
			 ++end) {
			if (strcmp(aPersons[end].folder, aPersons[best].folder) < 0) {
				best = end;
			}
		}
		for (size_t i = start; i < end; ++i) {
			if (i == best) {
				continue; // the one that is kept
			}
			TraceWarning(
				aTrace,
				"%s/data.json: skipped, Person Id %lld is also used by %s/data.json",
				aPersons[i].folder,
				aPersons[i].person.id,
				aPersons[best].folder);
			*aWarned = true;
		}
		aPersons[kept] = aPersons[best];
		++kept;
	}
	return kept;
}

static size_t
PrivFindIndex(const ContextPerson* aPersons, size_t aCount, FT_PersonId aPerson) {
	size_t low = 0;
	size_t high = aCount;
	while (low < high) {
		const size_t middle = low + ((high - low) / 2);
		if (aPersons[middle].person.id < aPerson) {
			low = middle + 1;
		} else {
			high = middle;
		}
	}
	return low < aCount && aPersons[low].person.id == aPerson ? low : aCount;
}

const ContextPerson*
FindPerson(const FT_Context* aContext, FT_PersonId aPerson) {
	const size_t index = PrivFindIndex(aContext->persons, aContext->personCount, aPerson);
	return index < aContext->personCount ? &aContext->persons[index] : NULL;
}

// ----- Relations of each Person

// Counts the Relations of each Person into aCounts, and reports Relations to unknown Persons.
static bool
PrivCountRelations(
	const FT_Context* aContext,
	const FT_Relation* aRelations,
	size_t aRelationCount,
	size_t* aCounts,
	FT_Trace* aTrace) {
	bool warned = false;
	for (size_t i = 0; i < aRelationCount; ++i) {
		const FT_Relation* relation = &aRelations[i];
		const size_t count = aContext->personCount;
		const size_t first = PrivFindIndex(aContext->persons, count, relation->first);
		const size_t second = PrivFindIndex(aContext->persons, count, relation->second);
		if (first == count || second == count) {
			TraceWarning(
				aTrace,
				"%s: Relation %lld,%lld names the unknown Person Id %lld, left out",
				theRelationsFile,
				relation->first,
				relation->second,
				first == count ? relation->first : relation->second);
			warned = true;
			continue;
		}
		++aCounts[first];
		if (second != first) {
			++aCounts[second];
		}
	}
	return warned;
}

static bool
PrivAllocateRelations(Arena* aArena, FT_Context* aContext, const size_t* aCounts) {
	for (size_t i = 0; i < aContext->personCount; ++i) {
		aContext->persons[i].relationCount = 0;
		aContext->persons[i].relations = NULL;
		if (aCounts[i] > 0) {
			aContext->persons[i].relations =
				ArenaAllocate(aArena, aCounts[i] * sizeof(FT_Relation));
			if (aContext->persons[i].relations == NULL) {
				return false;
			}
		}
	}
	return true;
}

static void
PrivAddRelation(ContextPerson* aPerson, const FT_Relation* aRelation) {
	// The array was allocated by this file, so writing to it is safe.
	FT_Relation* relations = (FT_Relation*)(uintptr_t)aPerson->relations;
	assert(relations != NULL && "allocated for every Person with Relations");
	relations[aPerson->relationCount] = *aRelation;
	++aPerson->relationCount;
}

// Gives each Person a view of the Relations that contain it, collected once at creation.
static FT_Result
PrivDistributeRelations(
	Arena* aArena,
	Arena* aScratch,
	FT_Context* aContext,
	const FT_Relation* aRelations,
	size_t aRelationCount,
	FT_Trace* aTrace) {
	size_t* counts = ArenaAllocate(aScratch, (aContext->personCount + 1) * sizeof(size_t));
	if (counts == NULL) {
		return FT_ERROR_OUT_OF_MEMORY;
	}
	(void)memset(counts, 0, (aContext->personCount + 1) * sizeof(size_t));
	const bool warned = PrivCountRelations(aContext, aRelations, aRelationCount, counts, aTrace);
	if (!PrivAllocateRelations(aArena, aContext, counts)) {
		return FT_ERROR_OUT_OF_MEMORY;
	}
	const size_t count = aContext->personCount;
	for (size_t i = 0; i < aRelationCount; ++i) {
		const size_t first = PrivFindIndex(aContext->persons, count, aRelations[i].first);
		const size_t second = PrivFindIndex(aContext->persons, count, aRelations[i].second);
		if (first == count || second == count) {
			continue;
		}
		const FT_Relation relation = PrivCompleteRelation(&aRelations[i]);
		PrivAddRelation(&aContext->persons[first], &relation);
		if (second != first) {
			PrivAddRelation(&aContext->persons[second], &relation);
		}
	}
	return warned ? FT_OK_WITH_WARNINGS : FT_OK;
}

// ----- Creation

// Copies the Persons into a sorted index without duplicate Person Ids. The Persons are copied, so
// that a failed creation leaves the Builder's own array as it was.
static FT_Result
PrivIndexPersons(
	Arena* aArena,
	Arena* aScratch,
	FT_Context* aContext,
	const ContextPerson* aPersons,
	size_t aPersonCount,
	FT_Trace* aTrace) {
	if (aPersonCount > (SIZE_MAX / sizeof(ContextPerson)) - 1) {
		return FT_ERROR_OUT_OF_MEMORY;
	}
	const size_t size = (aPersonCount + 1) * sizeof(ContextPerson);
	ContextPerson* persons = ArenaAllocate(aArena, size);
	ContextPerson* work = ArenaAllocate(aScratch, size);
	if (persons == NULL || work == NULL) {
		return FT_ERROR_OUT_OF_MEMORY;
	}
	if (aPersonCount > 0) {
		(void)memcpy(persons, aPersons, aPersonCount * sizeof(ContextPerson));
	}
	PrivSort(persons, work, aPersonCount);
	bool warned = false;
	aContext->persons = persons;
	aContext->personCount = PrivRemoveDuplicates(persons, aPersonCount, aTrace, &warned);
	return warned ? FT_OK_WITH_WARNINGS : FT_OK;
}

static FT_Result
PrivCombine(FT_Result aFirst, FT_Result aSecond) {
	if (aFirst != FT_OK && aFirst != FT_OK_WITH_WARNINGS) {
		return aFirst;
	}
	if (aSecond != FT_OK) {
		return aSecond;
	}
	return aFirst;
}

FT_Result
CreateContext(
	Arena* aArena,
	Arena* aScratch,
	const ContextPerson* aPersons,
	size_t aPersonCount,
	const FT_Relation* aRelations,
	size_t aRelationCount,
	FT_Context** aOutContext,
	FT_Trace* aTrace) {
	TraceOpen(aTrace, "CreateContext", "%zu Persons, %zu Relations", aPersonCount, aRelationCount);
	FT_Context* context = ArenaAllocate(aArena, sizeof(FT_Context));
	if (context == NULL) {
		return TraceClose(aTrace, FT_ERROR_OUT_OF_MEMORY);
	}
	(void)memset(context, 0, sizeof(*context));
	TraceStep(aTrace, "sorting the Persons by Person Id, keeping one of each Person Id");
	FT_Result result = PrivIndexPersons(aArena, aScratch, context, aPersons, aPersonCount, aTrace);
	if (result == FT_OK || result == FT_OK_WITH_WARNINGS) {
		TraceStep(aTrace, "giving each of %zu Persons its Relations", context->personCount);
		result = PrivCombine(
			result,
			PrivDistributeRelations(aArena, aScratch, context, aRelations, aRelationCount, aTrace));
	}
	// The Resolver goes here: Parents, Children, Siblings, Partners, Families and Generations,
	// computed from the index and the Relations (docs/workspace/implementers_notes.md#resolver).
	if (result != FT_OK && result != FT_OK_WITH_WARNINGS) {
		return TraceClose(aTrace, result);
	}
	*aOutContext = context;
	return TraceClose(aTrace, result);
}

FT_Result
FT_Free(FT_Context* aContext, FT_Trace* aOutTrace) {
	TraceOpen(aOutTrace, "FT_Free", "");
	if (aContext == NULL) {
		TraceError(aOutTrace, "the Context is NULL");
		return TraceEndCall(aOutTrace, FT_ERROR_INVALID_ARGUMENT);
	}
	// The Context lives in its own Arena, so the Arena is copied out before it is freed.
	Arena arena = aContext->arena;
	ArenaFree(&arena);
	return TraceEndCall(aOutTrace, FT_OK);
}
