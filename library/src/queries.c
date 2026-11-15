// Queries: views into the Context and caller buffers (docs/decisions/adr_0004.md,
// docs/decisions/adr_0009.md). Stubs: every function returns FT_ERROR_NOT_SUPPORTED until its work
// package implements it.

#include <family_tree/api.h>
#include <stdbool.h>
#include <stddef.h>

// The stubs write no output yet; remove this block with the last stub.
// NOLINTBEGIN(readability-non-const-parameter)

FT_Result
FT_GetPersonCount(const FT_Context* aContext, size_t* aOutCount, FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aOutCount;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_GetFamilyCount(const FT_Context* aContext, size_t* aOutCount, FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aOutCount;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_GetPerson(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const FT_Person** aOutPerson,
	FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aPerson;
	(void)aOutPerson;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_GetRelations(
	const FT_Context* aContext,
	FT_PersonId aPerson,
	const FT_Relation** aOutRelations,
	size_t* aOutCount,
	FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aPerson;
	(void)aOutRelations;
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
FT_IsDefaultString(
	const FT_Context* aContext,
	const char* aText,
	bool* aOutMissing,
	FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aText;
	(void)aOutMissing;
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
