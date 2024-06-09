#include "internal_types.h"

#include <stdlib.h>

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
ProviderComposit_Copy(IDataProvider* aThis) {
	return aThis;
}

void
ProviderComposit_GetPerson(IDataProvider* aThis, PersonId aId, Person* aOutPerson) {
	ProviderComposit* self = (ProviderComposit*)aThis;

	for (size_t i = 0; i < self->myTranslationSize; i++) {
		if (self->myTranslations[i].externalId != aId) {
			continue;
		}

		IDataProvider* provider = self->myTranslations[i].dataProvider;
		provider->GetPerson(provider, self->myTranslations[i].internalId, aOutPerson);
		aOutPerson->id = self->myTranslations[i].externalId;
		return;
	}
}

void
ProviderComposit_PlayPerson(IDataProvider* aThis, PersonId aId) {
	ProviderComposit* self = (ProviderComposit*)aThis;

	for (size_t i = 0; i < self->myTranslationSize; i++) {
		if (self->myTranslations[i].externalId != aId) {
			continue;
		}

		IDataProvider* provider = self->myTranslations[i].dataProvider;
		provider->PlayPerson(provider, self->myTranslations[i].internalId);
		return;
	}
}

void
ProviderComposit_ShowImages(IDataProvider* aThis, PersonId aId) {
	ProviderComposit* self = (ProviderComposit*)aThis;

	for (size_t i = 0; i < self->myTranslationSize; i++) {
		if (self->myTranslations[i].externalId != aId) {
			continue;
		}

		IDataProvider* provider = self->myTranslations[i].dataProvider;
		provider->ShowImages(provider, self->myTranslations[i].internalId);
		return;
	}
}

size_t
ProviderComposit_GetAllIdsCount(IDataProvider* aThis) {
	ProviderComposit* self = (ProviderComposit*)aThis;

	return self->myTranslationSize;
}

void
ProviderComposit_GetAllIds(IDataProvider* aThis, PersonId* aOutId) {
	ProviderComposit* self = (ProviderComposit*)aThis;

	for (size_t i = 0; i < self->myTranslationSize; i++) {
		aOutId[i] = self->myTranslations[i].externalId;
	}
}

size_t
ProviderComposit_GetAllRelationsOfIdCount(IDataProvider* aThis, PersonId aId) {
	ProviderComposit* self = (ProviderComposit*)aThis;

	for (size_t i = 0; i < self->myTranslationSize; i++) {
		if (self->myTranslations[i].externalId != aId) {
			continue;
		}

		IDataProvider* provider = self->myTranslations[i].dataProvider;
		return provider->GetAllRelationsOfIdCount(provider, self->myTranslations[i].internalId);
	}
	return 0;
}

void
ProviderComposit_GetAllRelationsOfId(IDataProvider* aThis, PersonId aId, Relation* aOutRelation) {
	ProviderComposit* self = (ProviderComposit*)aThis;

	IDataProvider* provider = NULL;
	size_t count = 0;
	for (size_t i = 0; i < self->myTranslationSize; i++) {
		if (self->myTranslations[i].externalId != aId) {
			continue;
		}

		provider = self->myTranslations[i].dataProvider;
		count = provider->GetAllRelationsOfIdCount(provider, self->myTranslations[i].internalId);
		provider->GetAllRelationsOfId(provider, self->myTranslations[i].internalId, aOutRelation);
		break;
	}

	for (size_t i = 0; i < count; i++) {
		for (size_t j = 0; j < self->myTranslationSize; j++) {
			if (self->myTranslations[j].dataProvider != provider) {
				continue;
			}
			if (aOutRelation[i].id1 == self->myTranslations[j].internalId) {
				aOutRelation[i].id1 = self->myTranslations[j].externalId;
			}
			if (aOutRelation[i].id2 == self->myTranslations[j].internalId) {
				aOutRelation[i].id2 = self->myTranslations[j].externalId;
			}
		}
	}
}

RelationType
ProviderComposit_GetRelationType(IDataProvider* aThis, Relation aRelation) {
	ProviderComposit* self = (ProviderComposit*)aThis;

	IDataProvider* dataProvider = NULL;

	for (size_t i = 0; i < self->myTranslationSize; i++) {
		if (aRelation.id1 == self->myTranslations[i].externalId) {
			aRelation.id1 = self->myTranslations[i].internalId;
			if (dataProvider != NULL) {
				break;
			}
			dataProvider = self->myTranslations[i].dataProvider;
		}
		if (aRelation.id2 == self->myTranslations[i].externalId) {
			aRelation.id2 = self->myTranslations[i].internalId;
			if (dataProvider != NULL) {
				break;
			}
			dataProvider = self->myTranslations[i].dataProvider;
		}
	}
	if (dataProvider == NULL) {
		return RelationType_Unrestricted;
	}

	return dataProvider->GetRelationType(dataProvider, aRelation);
}

void
ProviderComposit_Free(IDataProvider* aThis) {
	ProviderComposit* self = (ProviderComposit*)aThis;

	for (size_t i = 0; i < self->myDataProviderSize; i++) {
		self->myDataProviders[i]->Free(self->myDataProviders[i]);
	}

	free(self->myDataProviders);
	free(self->myTranslations);
	free(self);
}

ProviderComposit*
CreateProviderComposit() {
	ProviderComposit* result = calloc(1, sizeof(ProviderComposit));

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
AddDataProviderToArray(ProviderComposit* self, IDataProvider* aProvider) {
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
ExtendTranslationTable(ProviderComposit* self, IDataProvider* aProvider) {
	size_t internalIdSize = aProvider->GetAllIdsCount(aProvider);

	IdTranslation* newArray = realloc(
		self->myTranslations,
		sizeof(IdTranslation) * (self->myTranslationSize + internalIdSize));
	if (newArray == NULL) {
		return;
	}

	PersonId* internalIds = calloc(internalIdSize, sizeof(PersonId));
	aProvider->GetAllIds(aProvider, internalIds);
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
ProviderComposit_AddDataProvider(ProviderComposit* aThis, IDataProvider* aProvider) {
	AddDataProviderToArray(aThis, aProvider);
	ExtendTranslationTable(aThis, aProvider);
}

IDataProvider*
ProviderComposit_Cast(ProviderComposit* aThis) {
	return &aThis->interface;
}
