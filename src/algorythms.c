#include "algorythms.h"

#include "patch.h"

#include <stdlib.h>

typedef struct PersonMeta_t {
	PersonId myPersonId;

	size_t myParentCount;
	PersonMeta** myParents;
	size_t myChildCount;
	PersonMeta** myChilds;

	int myGeneration;
} PersonMeta;

void ResolveGenerationsRecursively(PersonMeta* element, int minGeneration, int aFromChild);
void SquashUp(PersonMeta* element);

void
ResolveGenerationsRecursively(PersonMeta* element, int minGeneration, int aFromChild) {
	if (element->myGeneration < minGeneration) {
		element->myGeneration = minGeneration;

		for (size_t i = 0; i < element->myChildCount; i++) {
			ResolveGenerationsRecursively(element->myChilds[i], element->myGeneration + 1, 0);
		}
	}
	if (aFromChild != 1) {
		SquashUp(element); // might be permanent loop
	}
}

void
SquashUp(PersonMeta* element) {
	if (element->myParentCount == 0) {
		return;
	}

	// alle Parents auf den höhesten wert setzen
	int maxNumber = -1;
	for (size_t i = 0; i < element->myParentCount; i++) {
		maxNumber = maxNumber > element->myParents[i]->myGeneration
						? maxNumber
						: element->myParents[i]->myGeneration;
	}

	// Childrens von Parents updaten lassen
	for (size_t i = 0; i < element->myParentCount; i++) {
		ResolveGenerationsRecursively(element->myParents[i], maxNumber, 1);
	}
}

void
ResolveGraph(MetaData* aMetaData) {
	for (size_t i = 0; i < aMetaData->myPersonCount; i++) {
		if (aMetaData->myPersons[i].myParentCount == 0) {
			ResolveGenerationsRecursively(aMetaData->myPersons + i, 0, 0);
		}
	}
}

MetaData
CreateMetaData(IDataProvider* aDataProvider) {
	MetaData data;

	data.myPersonCount = aDataProvider->GetAllIdsCount(aDataProvider);
	data.myPersons = calloc(data.myPersonCount, sizeof(PersonMeta));
	PersonId* ids = calloc(data.myPersonCount, sizeof(PersonId));
	aDataProvider->GetAllIds(aDataProvider, ids);

	for (size_t i = 0; i < data.myPersonCount; i++) {
		data.myPersons[i].myPersonId = ids[i];
		data.myPersons[i].myGeneration = -1;

		size_t count = aDataProvider->GetAllRelationsOfIdCount(aDataProvider, ids[i]);
		Relation* relations = calloc(count, sizeof(Relation));
		aDataProvider->GetAllRelationsOfId(aDataProvider, ids[i], relations);

		for (size_t j = 0; j < count; j++) {
			RelationType relation = aDataProvider->GetRelationType(aDataProvider, relations[j]);
			if (relation == RelationType_Unrestricted) {
				continue;
			}
			PersonId otherId = relations[j].id1 == ids[i] ? relations[j].id2 : relations[j].id1;
			if (relations[j].id2 == ids[i]) {
				if (relation == RelationType_StrictlyHigher) {
					relation = RelationType_StrictlyLower;
				} else {
					relation = RelationType_StrictlyHigher;
				}
			}
			size_t otherIndex = 0;
			for (size_t k = 0; k < data.myPersonCount; k++) {
				if (otherId == ids[k]) {
					otherIndex = k;
					break;
				}
			}
			if (relation == RelationType_StrictlyHigher) {
				data.myPersons[i].myParentCount++;
				data.myPersons[i].myParents = realloc(
					data.myPersons[i].myParents,
					data.myPersons[i].myParentCount * sizeof(PersonMeta*));

				data.myPersons[i].myParents[data.myPersons[i].myParentCount - 1] =
					data.myPersons + otherIndex;
			} else {
				data.myPersons[i].myChildCount++;
				data.myPersons[i].myChilds = realloc(
					data.myPersons[i].myChilds,
					data.myPersons[i].myChildCount * sizeof(PersonMeta*));

				data.myPersons[i].myChilds[data.myPersons[i].myChildCount - 1] =
					data.myPersons + otherIndex;
			}
		}

		free(relations);
	}

	ResolveGraph(&data);
	free(ids);
	return data;
}

void
FreeMetaData(MetaData* aMetaData) {
	if (aMetaData->myPersonCount == 0) {
		return;
	}

	for (size_t i = 0; i < aMetaData->myPersonCount; i++) {
		free(aMetaData->myPersons[i].myParents);
		free(aMetaData->myPersons[i].myChilds);
	}
	free(aMetaData->myPersons);
	aMetaData->myPersonCount = 0;
}

int
ComputeRelativeGeneration(MetaData aMetaData, PersonId aId1, PersonId aId2) {
	int generationOfId1 = -2;
	int generationOfId2 = -2;
	for (size_t i = 0; i < aMetaData.myPersonCount; i++) {
		if (aMetaData.myPersons[i].myPersonId == aId1) {
			generationOfId1 = aMetaData.myPersons[i].myGeneration;
			if (generationOfId2 != -2) {
				break;
			}
		}
		if (aMetaData.myPersons[i].myPersonId == aId2) {
			generationOfId2 = aMetaData.myPersons[i].myGeneration;
			if (generationOfId1 != -2) {
				break;
			}
		}
	}
	return generationOfId2 - generationOfId1;
}
