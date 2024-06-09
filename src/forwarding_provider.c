#include "internal_types.h"

#include <stdlib.h>

typedef struct {
	IDataProvider interface;

	IRelationals* myRelations;
	IPersonals* myPersonals;
} ForwardingProvider;

IDataProvider*
ForwardingProvider_Copy(IDataProvider* aThis) {
	return aThis;
}

void
ForwardingProvider_GetPerson(IDataProvider* aThis, PersonId aId, Person* aOutPerson) {
	ForwardingProvider* self = (ForwardingProvider*)aThis;

	self->myPersonals->GetPerson(self->myPersonals, aId, aOutPerson);
}

void
ForwardingProvider_PlayPerson(IDataProvider* aThis, PersonId aId) {
	ForwardingProvider* self = (ForwardingProvider*)aThis;

	self->myPersonals->PlayPerson(self->myPersonals, aId);
}

void
ForwardingProvider_ShowImages(IDataProvider* aThis, PersonId aId) {
	ForwardingProvider* self = (ForwardingProvider*)aThis;

	self->myPersonals->ShowImages(self->myPersonals, aId);
}

size_t
ForwardingProvider_GetAllIdsCount(IDataProvider* aThis) {
	ForwardingProvider* self = (ForwardingProvider*)aThis;

	size_t personalCount = self->myPersonals->GetAllIdsCount(self->myPersonals);
	PersonId* personalIds = calloc(personalCount, sizeof(PersonId));
	self->myPersonals->GetAllIds(self->myPersonals, personalIds);

	size_t relationCount = self->myRelations->GetAllIdsCount(self->myRelations);
	PersonId* relationIds = calloc(relationCount, sizeof(PersonId));
	self->myRelations->GetAllIds(self->myRelations, relationIds);

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
ForwardingProvider_GetAllIds(IDataProvider* aThis, PersonId* aOutId) {
	ForwardingProvider* self = (ForwardingProvider*)aThis;

	size_t personalCount = self->myPersonals->GetAllIdsCount(self->myPersonals);
	self->myPersonals->GetAllIds(self->myPersonals, aOutId);

	size_t relationCount = self->myRelations->GetAllIdsCount(self->myRelations);
	PersonId* relationIds = calloc(relationCount, sizeof(PersonId));
	self->myRelations->GetAllIds(self->myRelations, relationIds);

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
ForwardingProvider_GetAllRelationsOfIdCount(IDataProvider* aThis, PersonId aId) {
	ForwardingProvider* self = (ForwardingProvider*)aThis;

	return self->myRelations->GetAllRelationsOfIdCount(self->myRelations, aId);
}

void
ForwardingProvider_GetAllRelationsOfId(IDataProvider* aThis, PersonId aId, Relation* aOutRelation) {
	ForwardingProvider* self = (ForwardingProvider*)aThis;

	self->myRelations->GetAllRelationsOfId(self->myRelations, aId, aOutRelation);
}

RelationType
ForwardingProvider_GetRelationType(IDataProvider* aThis, Relation aRelation) {
	ForwardingProvider* self = (ForwardingProvider*)aThis;

	return self->myRelations->GetRelationType(self->myRelations, aRelation);
}

void
ForwardingProvider_Free(IDataProvider* aThis) {
	ForwardingProvider* self = (ForwardingProvider*)aThis;

	self->myPersonals->Free(self->myPersonals);
	self->myRelations->Free(self->myRelations);
	free(self);
}

IDataProvider*
CreateForwardingProvider(IRelationals* aRelations, IPersonals* aPersonals) {
	ForwardingProvider* result = calloc(1, sizeof(ForwardingProvider));

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
