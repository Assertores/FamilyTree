#include "internal_types.h"
#include "patch.h"

#include <stdlib.h>

typedef struct {
	IDataProvider interface;

	IRelationals* myRelations;
	IPersonals* myPersonals;
} ForwardingProvider;

IDataProvider*
ForwardingProvider_Copy(IDataProvider* aThis, ITrace* aTrace) {
	return aThis;
}

Person
ForwardingProvider_GetPerson(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	ForwardingProvider* self = (ForwardingProvider*)aThis;

	return self->myPersonals->GetPerson(self->myPersonals, aId, aTrace);
}

void
ForwardingProvider_PlayPerson(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	ForwardingProvider* self = (ForwardingProvider*)aThis;

	self->myPersonals->PlayPerson(self->myPersonals, aId, aTrace);
}

void
ForwardingProvider_ShowImages(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	ForwardingProvider* self = (ForwardingProvider*)aThis;

	self->myPersonals->ShowImages(self->myPersonals, aId, aTrace);
}

size_t
ForwardingProvider_GetAllIdsCount(IDataProvider* aThis, ITrace* aTrace) {
	ForwardingProvider* self = (ForwardingProvider*)aThis;

	size_t personalCount = self->myPersonals->GetAllIdsCount(self->myPersonals, aTrace);
	PersonId* personalIds = calloc(personalCount, sizeof(PersonId));
	self->myPersonals->GetAllIds(self->myPersonals, personalIds, aTrace);

	size_t relationCount = self->myRelations->GetAllIdsCount(self->myRelations, aTrace);
	PersonId* relationIds = calloc(relationCount, sizeof(PersonId));
	self->myRelations->GetAllIds(self->myRelations, relationIds, aTrace);

	size_t result = personalCount + relationCount;
	for (int i = 0; i < relationCount; i++) {
		for (int j = 0; j < personalCount; j++) {
			if (relationIds[i] == personalIds[j]) {
				result--;
				break;
			}
		}
	}

	free(personalIds);
	free(relationIds);
	return result;
}

void
ForwardingProvider_GetAllIds(IDataProvider* aThis, PersonId* aOutId, ITrace* aTrace) {
	ForwardingProvider* self = (ForwardingProvider*)aThis;

	size_t personalCount = self->myPersonals->GetAllIdsCount(self->myPersonals, aTrace);
	self->myPersonals->GetAllIds(self->myPersonals, aOutId, aTrace);

	size_t relationCount = self->myRelations->GetAllIdsCount(self->myRelations, aTrace);
	PersonId* relationIds = calloc(relationCount, sizeof(PersonId));
	self->myRelations->GetAllIds(self->myRelations, relationIds, aTrace);

	int newCount = 0;
	for (int i = 0, duplicate = 0; i < relationCount; i++, duplicate = 0) {
		for (int j = 0; j < personalCount; j++) {
			if (relationIds[i] == aOutId[j]) {
				duplicate = 1;
				break;
			}
		}
		if (!duplicate) {
			aOutId[personalCount + newCount] = relationIds[i];
			newCount++;
		}
	}

	free(relationIds);
}

size_t
ForwardingProvider_GetAllRelationsOfIdCount(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	ForwardingProvider* self = (ForwardingProvider*)aThis;

	return self->myRelations->GetAllRelationsOfIdCount(self->myRelations, aId, aTrace);
}

void
ForwardingProvider_GetAllRelationsOfId(
	IDataProvider* aThis, PersonId aId, Relation* aOutRelation, ITrace* aTrace) {
	ForwardingProvider* self = (ForwardingProvider*)aThis;

	self->myRelations->GetAllRelationsOfId(self->myRelations, aId, aOutRelation, aTrace);
}

RelationType
ForwardingProvider_GetRelationType(IDataProvider* aThis, Relation aRelation, ITrace* aTrace) {
	ForwardingProvider* self = (ForwardingProvider*)aThis;

	return self->myRelations->GetRelationType(self->myRelations, aRelation, aTrace);
}

void
ForwardingProvider_Free(IDataProvider* aThis, ITrace* aTrace) {
	ForwardingProvider* self = (ForwardingProvider*)aThis;

	self->myPersonals->Free(self->myPersonals, aTrace);
	self->myRelations->Free(self->myRelations, aTrace);
	free(self);
}

IDataProvider*
CreateForwardingProvider(IRelationals* aRelations, IPersonals* aPersonals, ITrace* aTrace) {
	ITrace* trace = aTrace->CreateSubTrace(aTrace, "Construct Forwardigng Data Provider");
	trace->AddEvent(trace, "Allocate memory");
	ForwardingProvider* result = calloc(1, sizeof(ForwardingProvider));

	trace->AddEvent(trace, "Set Dispatch Table");
	result->interface.Copy = ForwardingProvider_Copy;
	result->interface.GetPerson = ForwardingProvider_GetPerson;
	result->interface.PlayPerson = ForwardingProvider_PlayPerson;
	result->interface.ShowImages = ForwardingProvider_ShowImages;
	result->interface.GetAllIdsCount = ForwardingProvider_GetAllIdsCount;
	result->interface.GetAllIds = ForwardingProvider_GetAllIds;
	result->interface.GetAllRelationsOfIdCount = ForwardingProvider_GetAllRelationsOfIdCount;
	result->interface.GetAllRelationsOfId = ForwardingProvider_GetAllRelationsOfId;
	result->interface.GetRelationType = ForwardingProvider_GetRelationType;
	result->interface.Free = ForwardingProvider_Free;

	result->myPersonals = aPersonals;
	result->myRelations = aRelations;

	return &result->interface;
}
