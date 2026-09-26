#include "setup.h"

#include <family_tree/api.h>
#include <stddef.h>

#define SET_DEBUG()        \
	DebuggingData backing; \
	if (aDebug == NULL) {  \
		aDebug = &backing; \
	}

#define CHECKS SET_DEBUG();

FT_API Context*
Create(Utf8_String aPath, DebuggingData* aDebug) {
	ContextBuilder builder = StartCreate(aPath, aDebug);
	while (builder.question != QuestionType_ContextIsReady) {
		builder.question *= -1;
		Continue(builder, aDebug);
	}
	return builder.context;
}

FT_API ContextBuilder
StartCreate(Utf8_String aPath, DebuggingData* aDebug) {
	SET_DEBUG();
	return DoStart(aPath, aDebug);
}

FT_API ContextBuilder
Continue(ContextBuilder aPreviousStep, DebuggingData* aDebug) {
	SET_DEBUG();
	return DoContinue(aPreviousStep, aDebug);
}

FT_API Context*
Merge(Context* aContext1, Context* aContext2, DebuggingData* aDebug) {
	SET_DEBUG();
}

FT_API void
Free(Context* aContext, DebuggingData* aDebug) {
	SET_DEBUG();
}

FT_API Person
GetPerson(Context* aContext, PersonId aId, DebuggingData* aDebug) {
	SET_DEBUG();
}

FT_API void
PlayPerson(Context* aContext, PersonId aId, DebuggingData* aDebug) {
	SET_DEBUG();
}

FT_API void
ShowImagesOfPerson(Context* aContext, PersonId aId, DebuggingData* aDebug) {
	SET_DEBUG();
}

FT_API Person*
GetPersonsMatchingPattern(
	Context* aContext,
	Person aPrototype,
	size_t aMinMatches,
	size_t* aOutPersonsCount,
	DebuggingData* aDebug) {
	SET_DEBUG();
}

FT_API Relation*
GetPersonRelations(
	Context* aContext,
	PersonId aId,
	size_t* aOutRelationsCount,
	DebuggingData* aDebug) {
	SET_DEBUG();
}

FT_API int
GetRelativeGeneration(
	Context* aContext,
	PersonId aRefId,
	PersonId aTargetId,
	DebuggingData* aDebug) {
	SET_DEBUG();
}

FT_API PersonId*
GetPartners(Context* aContext, PersonId aId, size_t* aOutParterCount, DebuggingData* aDebug) {
	SET_DEBUG();
}

FT_API PersonId*
GetSiblings(Context* aContext, PersonId aId, size_t* aOutSiblingCount, DebuggingData* aDebug) {
	SET_DEBUG();
}

FT_API PersonId*
GetCommonParents(
	Context* aContext,
	size_t aIdCount,
	PersonId* aIds,
	size_t* aOutParentCount,
	DebuggingData* aDebug) {
	SET_DEBUG();
}

FT_API PersonId*
GetCommonChildren(
	Context* aContext,
	size_t aIdCount,
	PersonId* aIds,
	size_t* aOutChildrenCount,
	DebuggingData* aDebug) {
	SET_DEBUG();
}

FT_API void
BuildGraph(
	Context* aContext,
	PersonId aId,
	size_t aDistance,
	GraphBuilderStrategy* aStrategy,
	DebuggingData* aDebug) {
	SET_DEBUG();
}

FT_API int
IsDefaultString(Context* aContext, const char* aStringToCheck) {}
