#include "internal_types.h"
#include "patch.h"

#include <stdlib.h>
#include <string.h>

typedef struct IdTranslation {
	PersonId externalId;
	IDataProvider* dataProvider;
	PersonId internalId;
} IdTranslation;

struct ProviderComposit {
	IDataProvider interface;

	size_t myTranslationSize;
	IdTranslation* myTranslations;

	size_t myDataProviderSize;
	IDataProvider** myDataProviders;
};

IDataProvider*
ProviderComposit_Copy(IDataProvider* aThis, ITrace* aTrace) {
	return aThis;
}

Person
ProviderComposit_GetPerson(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	ProviderComposit* self = (ProviderComposit*)aThis;

	for (size_t i = 0; i < self->myTranslationSize; i++) {
		if (self->myTranslations[i].externalId != aId) {
			continue;
		}

		IDataProvider* provider = self->myTranslations[i].dataProvider;
		Person result = provider->GetPerson(provider, self->myTranslations[i].internalId, aTrace);
		result.id = self->myTranslations[i].externalId;
		return result;
	}
	Person result;
	memset(&result, 0, sizeof(Person));
	return result;
}

void
ProviderComposit_PlayPerson(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	ProviderComposit* self = (ProviderComposit*)aThis;

	for (size_t i = 0; i < self->myTranslationSize; i++) {
		if (self->myTranslations[i].externalId != aId) {
			continue;
		}

		IDataProvider* provider = self->myTranslations[i].dataProvider;
		provider->PlayPerson(provider, self->myTranslations[i].internalId, aTrace);
		return;
	}
}

void
ProviderComposit_ShowImages(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	ProviderComposit* self = (ProviderComposit*)aThis;

	for (size_t i = 0; i < self->myTranslationSize; i++) {
		if (self->myTranslations[i].externalId != aId) {
			continue;
		}

		IDataProvider* provider = self->myTranslations[i].dataProvider;
		provider->ShowImages(provider, self->myTranslations[i].internalId, aTrace);
		return;
	}
}

size_t
ProviderComposit_GetAllIdsCount(IDataProvider* aThis, ITrace* aTrace) {
	ProviderComposit* self = (ProviderComposit*)aThis;

	return self->myTranslationSize;
}

void
ProviderComposit_GetAllIds(IDataProvider* aThis, PersonId* aOutId, ITrace* aTrace) {
	ProviderComposit* self = (ProviderComposit*)aThis;

	for (size_t i = 0; i < self->myTranslationSize; i++) {
		aOutId[i] = self->myTranslations[i].externalId;
	}
}

size_t
ProviderComposit_GetAllRelationsOfIdCount(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	ProviderComposit* self = (ProviderComposit*)aThis;

	for (size_t i = 0; i < self->myTranslationSize; i++) {
		if (self->myTranslations[i].externalId != aId) {
			continue;
		}

		IDataProvider* provider = self->myTranslations[i].dataProvider;
		return provider->GetAllRelationsOfIdCount(
			provider,
			self->myTranslations[i].internalId,
			aTrace);
	}
	return 0;
}

void
ProviderComposit_GetAllRelationsOfId(
	IDataProvider* aThis, PersonId aId, Relation* aOutRelation, ITrace* aTrace) {
	ProviderComposit* self = (ProviderComposit*)aThis;

	IDataProvider* provider = NULL;
	size_t count = 0;
	for (size_t i = 0; i < self->myTranslationSize; i++) {
		if (self->myTranslations[i].externalId != aId) {
			continue;
		}

		provider = self->myTranslations[i].dataProvider;
		count = provider->GetAllRelationsOfIdCount(
			provider,
			self->myTranslations[i].internalId,
			aTrace);
		provider->GetAllRelationsOfId(
			provider,
			self->myTranslations[i].internalId,
			aOutRelation,
			aTrace);
		break;
	}

	for (size_t i = 0; i < count; i++) {
		PersonId id1 = -1;
		PersonId id2 = -1;
		for (size_t j = 0; j < self->myTranslationSize; j++) {
			if (self->myTranslations[j].dataProvider != provider) {
				continue;
			}
			if (aOutRelation[i].id1 == self->myTranslations[j].internalId) {
				id1 = self->myTranslations[j].externalId;
			}
			if (aOutRelation[i].id2 == self->myTranslations[j].internalId) {
				id2 = self->myTranslations[j].externalId;
			}
		}
		aOutRelation[i].id1 = id1;
		aOutRelation[i].id2 = id2;
	}
}

RelationType
ProviderComposit_GetRelationType(IDataProvider* aThis, Relation aRelation, ITrace* aTrace) {
	ProviderComposit* self = (ProviderComposit*)aThis;

	IDataProvider* dataProvider = NULL;

	PersonId id1 = -1;
	PersonId id2 = -1;
	for (size_t i = 0; i < self->myTranslationSize; i++) {
		if (aRelation.id1 == self->myTranslations[i].externalId) {
			id1 = self->myTranslations[i].internalId;
			if (dataProvider != NULL) {
				break;
			}
			dataProvider = self->myTranslations[i].dataProvider;
		}
		if (aRelation.id2 == self->myTranslations[i].externalId) {
			id2 = self->myTranslations[i].internalId;
			if (dataProvider != NULL) {
				break;
			}
			dataProvider = self->myTranslations[i].dataProvider;
		}
	}
	aRelation.id1 = id1;
	aRelation.id2 = id2;
	if (dataProvider == NULL) {
		return RelationType_Unrestricted;
	}

	return dataProvider->GetRelationType(dataProvider, aRelation, aTrace);
}

void
ProviderComposit_Free(IDataProvider* aThis, ITrace* aTrace) {
	ProviderComposit* self = (ProviderComposit*)aThis;

	for (size_t i = 0; i < self->myDataProviderSize; i++) {
		self->myDataProviders[i]->Free(self->myDataProviders[i], aTrace);
	}

	free(self->myDataProviders);
	free(self->myTranslations);
	free(self);
}

ProviderComposit*
CreateProviderComposit(ITrace* aTrace) {
	ITrace* trace = aTrace->CreateSubTrace(aTrace, "Construct Provider Composit");
	trace->AddEvent(trace, "Allocate memory");
	ProviderComposit* result = calloc(1, sizeof(ProviderComposit));

	trace->AddEvent(trace, "Set Dispatch Table");
	result->interface.Copy = ProviderComposit_Copy;
	result->interface.GetPerson = ProviderComposit_GetPerson;
	result->interface.PlayPerson = ProviderComposit_PlayPerson;
	result->interface.ShowImages = ProviderComposit_ShowImages;
	result->interface.GetAllIdsCount = ProviderComposit_GetAllIdsCount;
	result->interface.GetAllIds = ProviderComposit_GetAllIds;
	result->interface.GetAllRelationsOfIdCount = ProviderComposit_GetAllRelationsOfIdCount;
	result->interface.GetAllRelationsOfId = ProviderComposit_GetAllRelationsOfId;
	result->interface.GetRelationType = ProviderComposit_GetRelationType;
	result->interface.Free = ProviderComposit_Free;

	return result;
}

void
AddDataProviderToArray(ProviderComposit* self, IDataProvider* aProvider, ITrace* aTrace) {
	IDataProvider** newArray =
		realloc(self->myDataProviders, sizeof(IRelationals*) * (self->myDataProviderSize + 1));
	if (newArray == NULL) {
		return;
	}

	newArray[self->myDataProviderSize] = aProvider;
	self->myDataProviderSize++;
	self->myDataProviders = newArray;
}

void
ExtendTranslationTable(ProviderComposit* self, IDataProvider* aProvider, ITrace* aTrace) {
	size_t internalIdSize = aProvider->GetAllIdsCount(aProvider, aTrace);

	IdTranslation* newArray = realloc(
		self->myTranslations,
		sizeof(IdTranslation) * (self->myTranslationSize + internalIdSize));
	if (newArray == NULL) {
		return;
	}

	PersonId* internalIds = calloc(internalIdSize, sizeof(PersonId));
	aProvider->GetAllIds(aProvider, internalIds, aTrace);
	for (size_t i = 0; i < internalIdSize; i++) {
		newArray[self->myTranslationSize + i].dataProvider = aProvider;
		newArray[self->myTranslationSize + i].internalId = internalIds[i];
		// NOTE: maybe the externalId field could be droped. i don't know yet.
		newArray[self->myTranslationSize + i].externalId = self->myTranslationSize + i;
	}

	self->myTranslationSize += internalIdSize;
	self->myTranslations = newArray;

	free(internalIds);
}

void
ProviderComposit_AddDataProvider(
	ProviderComposit* aThis, IDataProvider* aProvider, ITrace* aTrace) {
	AddDataProviderToArray(aThis, aProvider, aTrace);
	ExtendTranslationTable(aThis, aProvider, aTrace);
}

IDataProvider*
ProviderComposit_Cast(ProviderComposit* aThis) {
	return &aThis->interface;
}
