// Queries: views into the Context and caller buffers (docs/decisions/adr_0004.md,
// docs/decisions/adr_0009.md). Stubs: every function that still returns FT_ERROR_NOT_SUPPORTED
// waits for its work package.

#include "internal.h"

#include <assert.h>
#include <family_tree/api.h>
#include <stdbool.h>
#include <stddef.h>

FT_Result
FT_GetPersonCount(const FT_Context* aContext, size_t* aOutCount, FT_Trace* aOutTrace) {
	TraceOpen(aOutTrace, "FT_GetPersonCount", "");
	if (aContext == NULL || aOutCount == NULL) {
		TraceError(aOutTrace, "the Context or the count is NULL");
		return TraceEndCall(aOutTrace, FT_ERROR_INVALID_ARGUMENT);
	}
	*aOutCount = aContext->personCount;
	return TraceEndCall(aOutTrace, FT_OK);
}

// The Person with the Person Id, or NULL after reporting that it is unknown.
static const ContextPerson*
PrivFindForView(const FT_Context* aContext, FT_PersonId aPerson, FT_Trace* aOutTrace) {
	assert(aContext != NULL);
	const ContextPerson* person = FindPerson(aContext, aPerson);
	if (person == NULL) {
		TraceError(aOutTrace, "unknown Person Id %lld", aPerson);
	}
	return person;
}

FT_Result
FT_GetPerson(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const FT_Person** aOutPerson,
	FT_Trace* aOutTrace) {
	TraceOpen(aOutTrace, "FT_GetPerson", "%lld", aPerson);
	if (aContext == NULL || aOutPerson == NULL) {
		TraceError(aOutTrace, "the Context or the Person is NULL");
		return TraceEndCall(aOutTrace, FT_ERROR_INVALID_ARGUMENT);
	}
	const ContextPerson* person = PrivFindForView(aContext, aPerson, aOutTrace);
	if (person == NULL) {
		return TraceEndCall(aOutTrace, FT_ERROR_UNKNOWN_PERSON);
	}
	*aOutPerson = &person->person;
	return TraceEndCall(aOutTrace, FT_OK);
}

FT_Result
FT_GetRelations(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const FT_Relation** aOutRelations,
	size_t* aOutCount,
	FT_Trace* aOutTrace) {
	TraceOpen(aOutTrace, "FT_GetRelations", "%lld", aPerson);
	if (aContext == NULL || aOutRelations == NULL || aOutCount == NULL) {
		TraceError(aOutTrace, "the Context, the Relations or the count is NULL");
		return TraceEndCall(aOutTrace, FT_ERROR_INVALID_ARGUMENT);
	}
	const ContextPerson* person = PrivFindForView(aContext, aPerson, aOutTrace);
	if (person == NULL) {
		return TraceEndCall(aOutTrace, FT_ERROR_UNKNOWN_PERSON);
	}
	*aOutRelations = person->relations;
	*aOutCount = person->relationCount;
	return TraceEndCall(aOutTrace, FT_OK);
}

FT_Result
FT_IsDefaultString(
	const FT_Context* aContext,
	const char* aText,
	bool* aOutMissing,
	FT_Trace* aOutTrace) {
	TraceOpen(aOutTrace, "FT_IsDefaultString", "");
	if (aContext == NULL || aText == NULL || aOutMissing == NULL) {
		TraceError(aOutTrace, "the Context, the text or the result is NULL");
		return TraceEndCall(aOutTrace, FT_ERROR_INVALID_ARGUMENT);
	}
	*aOutMissing = IsDefaultString(aText);
	return TraceEndCall(aOutTrace, FT_OK);
}

// The stubs write no output yet; remove this block with the last stub.
// NOLINTBEGIN(readability-non-const-parameter)

FT_Result
FT_GetFamilyCount(const FT_Context* aContext, size_t* aOutCount, FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aOutCount;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_GetParents(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const FT_PersonId** aOutParents,
	size_t* aOutCount,
	FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aPerson;
	(void)aOutParents;
	(void)aOutCount;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_GetChildren(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const FT_PersonId** aOutChildren,
	size_t* aOutCount,
	FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aPerson;
	(void)aOutChildren;
	(void)aOutCount;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_GetSiblings(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const FT_PersonId** aOutSiblings,
	size_t* aOutCount,
	FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aPerson;
	(void)aOutSiblings;
	(void)aOutCount;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_GetPartners(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const FT_PersonId** aOutPartners,
	size_t* aOutCount,
	FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aPerson;
	(void)aOutPartners;
	(void)aOutCount;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_GetFamiliesOf(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const FT_FamilyIndex** aOutFamilies,
	size_t* aOutCount,
	FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aPerson;
	(void)aOutFamilies;
	(void)aOutCount;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_GetFamily(
	const FT_Context* aContext,
	FT_FamilyIndex aFamily,
	const FT_Family** aOutFamily,
	FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aFamily;
	(void)aOutFamily;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_GetPicturePaths(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const char* const** aOutPaths,
	size_t* aOutCount,
	FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aPerson;
	(void)aOutPaths;
	(void)aOutCount;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_GetRecordingPath(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const char** aOutPath,
	FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aPerson;
	(void)aOutPath;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_GetRelativeGeneration(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	FT_PersonId aReference,
	int* aOutGeneration,
	FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aPerson;
	(void)aReference;
	(void)aOutGeneration;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_Search(
	const FT_Context* aContext,
	const FT_Person* aPrototype,
	size_t aMinimumMatches,
	FT_PersonId* aOutFound,
	size_t aCapacity,
	size_t* aOutCount,
	FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aPrototype;
	(void)aMinimumMatches;
	(void)aOutFound;
	(void)aCapacity;
	(void)aOutCount;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_GetCommonParents(
	const FT_Context* aContext,
	const FT_PersonId* aPersons,
	size_t aPersonCount,
	FT_PersonId* aOutParents,
	size_t aCapacity,
	size_t* aOutCount,
	FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aPersons;
	(void)aPersonCount;
	(void)aOutParents;
	(void)aCapacity;
	(void)aOutCount;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_GetCommonChildren(
	const FT_Context* aContext,
	const FT_PersonId* aPersons,
	size_t aPersonCount,
	FT_PersonId* aOutChildren,
	size_t aCapacity,
	size_t* aOutCount,
	FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aPersons;
	(void)aPersonCount;
	(void)aOutChildren;
	(void)aCapacity;
	(void)aOutCount;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_GetFamilyTree(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	size_t aDistance,
	FT_TreePerson* aOutPersons,
	size_t aPersonCapacity,
	size_t* aOutPersonCount,
	FT_FamilyIndex* aOutFamilies,
	size_t aFamilyCapacity,
	size_t* aOutFamilyCount,
	FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aPerson;
	(void)aDistance;
	(void)aOutPersons;
	(void)aPersonCapacity;
	(void)aOutPersonCount;
	(void)aOutFamilies;
	(void)aFamilyCapacity;
	(void)aOutFamilyCount;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

// NOLINTEND(readability-non-const-parameter)
