#include "algorythms.h"

#include "patch.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct PersonMeta_t {
	PersonId myPersonId;

	size_t myParentCount;
	PersonMeta** myParents;
	size_t myChildCount;
	PersonMeta** myChilds;

	int myGeneration;
} PersonMeta;

void ResolveGenerationsRecursively(
	PersonMeta* element,
	int minGeneration,
	int aFromChild,
	ITrace* aTrace);
void SquashUp(PersonMeta* element, ITrace* aTrace);

void
ResolveGenerationsRecursively(
	PersonMeta* element,
	int minGeneration,
	int aFromChild,
	ITrace* aTrace) {
	char string[126];
	sprintf(string, "Resolving generations for person: %zu", element->myPersonId);
	aTrace->AddEvent(aTrace, string);

	if (element->myGeneration < minGeneration) {
		sprintf(
			string,
			"Person: %zu needs to move from generation: %d to generation: %d",
			element->myPersonId,
			element->myGeneration,
			minGeneration);
		aTrace->AddEvent(aTrace, string);

		element->myGeneration = minGeneration;

		ITrace* trace = aTrace->CreateSubTrace(aTrace, "Childrens");
		for (size_t i = 0; i < element->myChildCount; i++) {
			ResolveGenerationsRecursively(
				element->myChilds[i],
				element->myGeneration + 1,
				0,
				trace);
		}
		trace->Free(trace);
	}
	if (aFromChild != 1) {
		SquashUp(element, aTrace); // might be permanent loop
	}
}

void
SquashUp(PersonMeta* element, ITrace* aTrace) {
	if (element->myParentCount == 0) {
		return;
	}

	// alle Parents auf den h�hesten wert setzen
	int maxNumber = -1;
	for (size_t i = 0; i < element->myParentCount; i++) {
		maxNumber = maxNumber > element->myParents[i]->myGeneration
						? maxNumber
						: element->myParents[i]->myGeneration;
	}

	char string[126];
	sprintf(
		string,
		"squashing up parents of person: %zu to generation: %d",
		element->myPersonId,
		maxNumber);
	aTrace->AddEvent(aTrace, string);

	ITrace* trace = aTrace->CreateSubTrace(aTrace, "Parents");
	// Childrens von Parents updaten lassen
	for (size_t i = 0; i < element->myParentCount; i++) {
		ResolveGenerationsRecursively(element->myParents[i], maxNumber, 1, trace);
	}
	trace->Free(trace);
}

void
ResolveGraph(MetaData* aMetaData, ITrace* aTrace) {
	ITrace* trace = aTrace->CreateSubTrace(aTrace, "Resolve Graph");
	for (size_t i = 0; i < aMetaData->myPersonCount; i++) {
		if (aMetaData->myPersons[i].myParentCount == 0) {
			char string[126];
			sprintf(
				string,
				"Person: %zu has no parents and is therefore a starting point.",
				aMetaData->myPersons[i].myPersonId);
			trace->AddEvent(trace, string);

			ResolveGenerationsRecursively(aMetaData->myPersons + i, 0, 0, trace);
		}
	}
	trace->Free(trace);
}

MetaData
CreateMetaData(IDataProvider* aDataProvider, ITrace* aTrace) {
	MetaData data;
	char string[126];

	ITrace* trace = aTrace->CreateSubTrace(aTrace, "GatherData");

	data.myPersonCount = aDataProvider->GetAllIdsCount(aDataProvider, trace);
	data.myPersons = calloc(data.myPersonCount, sizeof(PersonMeta));
	PersonId* ids = calloc(data.myPersonCount, sizeof(PersonId));
	aDataProvider->GetAllIds(aDataProvider, ids, trace);

	for (size_t i = 0; i < data.myPersonCount; i++) {
		data.myPersons[i].myPersonId = ids[i];
		data.myPersons[i].myGeneration = -1;

		size_t count = aDataProvider->GetAllRelationsOfIdCount(aDataProvider, ids[i], trace);
		Relation* relations = calloc(count, sizeof(Relation));
		aDataProvider->GetAllRelationsOfId(aDataProvider, ids[i], relations, trace);

		for (size_t j = 0; j < count; j++) {
			RelationType relation =
				aDataProvider->GetRelationType(aDataProvider, relations[j], trace);
			if (relation == RelationType_Unrestricted) {
				sprintf(string, "%zu and %zu are unrelaited", relations[j].id1, relations[j].id2);
				trace->AddEvent(trace, string);
				continue;
			}
			PersonId otherId = relations[j].id1 == ids[i] ? relations[j].id2 : relations[j].id1;
			if (relations[j].id2 == ids[i]) {
				trace->AddEvent(trace, "fliped relation for conveniance");
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
	trace->Free(trace);

	ResolveGraph(&data, aTrace);
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
ComputeRelativeGeneration(MetaData aMetaData, PersonId aId1, PersonId aId2, ITrace* aTrace) {
	int generationOfId1 = -2;
	int generationOfId2 = -2;
	for (size_t i = 0; i < aMetaData.myPersonCount; i++) {
		if (aMetaData.myPersons[i].myPersonId == aId1) {
			aTrace->AddEvent(aTrace, "found first person");
			generationOfId1 = aMetaData.myPersons[i].myGeneration;
			if (generationOfId2 != -2) {
				break;
			}
		}
		if (aMetaData.myPersons[i].myPersonId == aId2) {
			aTrace->AddEvent(aTrace, "found second person");
			generationOfId2 = aMetaData.myPersons[i].myGeneration;
			if (generationOfId1 != -2) {
				break;
			}
		}
	}
	aTrace->AddEvent(aTrace, "calculate difference");
	return generationOfId2 - generationOfId1;
}

size_t
ComputePartnersMinimalCount(MetaData aMetaData, PersonId aId, ITrace* aTrace) {
	size_t childrenCount = 0;
	PersonMeta** childrens = NULL;
	for (size_t i = 0; i < aMetaData.myPersonCount; i++) {
		if (aMetaData.myPersons[i].myPersonId == aId) {
			childrenCount = aMetaData.myPersons[i].myChildCount;
			childrens = aMetaData.myPersons[i].myChilds;
			break;
		}
	}
	if (childrens == NULL) {
		return 0;
	}
	size_t sum = 0;
	for (size_t i = 0; i < childrenCount; i++) {
		// not counting aId as parrent
		sum += childrens[i]->myParentCount - 1;
	}
	return sum;
}

size_t
ComputePartners(MetaData aMetaData, PersonId aId, PersonId* aOutPartners, ITrace* aTrace) {
	size_t childrenCount = 0;
	PersonMeta** childrens = NULL;
	for (size_t i = 0; i < aMetaData.myPersonCount; i++) {
		if (aMetaData.myPersons[i].myPersonId == aId) {
			childrenCount = aMetaData.myPersons[i].myChildCount;
			childrens = aMetaData.myPersons[i].myChilds;
			break;
		}
	}
	if (childrens == NULL) {
		return 0;
	}
	size_t uniqueParents = 0;
	for (size_t i = 0; i < childrenCount; i++) {
		for (size_t indexParent = 0; indexParent < childrens[i]->myParentCount; indexParent++) {
			if (childrens[i]->myParents[indexParent]->myPersonId == aId) {
				continue;
			}
			int isUnique = 1;
			for (size_t j = 0; j < uniqueParents; j++) {
				if (childrens[i]->myParents[indexParent]->myPersonId == aOutPartners[j]) {
					isUnique = 0;
					break;
				}
			}
			if (isUnique != 0) {
				aOutPartners[uniqueParents] = childrens[i]->myParents[indexParent]->myPersonId;
				uniqueParents++;
			}
		}
	}
	return uniqueParents;
}

size_t
ComputeSiblingsMinimalCount(MetaData aMetaData, PersonId aId, ITrace* aTrace) {
	size_t parentCount = 0;
	PersonMeta** parents = NULL;
	for (size_t i = 0; i < aMetaData.myPersonCount; i++) {
		if (aMetaData.myPersons[i].myPersonId == aId) {
			parentCount = aMetaData.myPersons[i].myParentCount;
			parents = aMetaData.myPersons[i].myParents;
			break;
		}
	}
	if (parents == NULL) {
		return 0;
	}
	size_t sum = 0;
	for (size_t i = 0; i < parentCount; i++) {
		// not counting aId as parrent
		sum += parents[i]->myChildCount - 1;
	}
	return sum;
}

size_t
ComputeSiblings(MetaData aMetaData, PersonId aId, PersonId* aOutSiblings, ITrace* aTrace) {
	size_t childrenCount = 0;
	PersonMeta** childrens = NULL;
	for (size_t i = 0; i < aMetaData.myPersonCount; i++) {
		if (aMetaData.myPersons[i].myPersonId == aId) {
			childrenCount = aMetaData.myPersons[i].myChildCount;
			childrens = aMetaData.myPersons[i].myChilds;
			break;
		}
	}
	if (childrens == NULL) {
		return 0;
	}
	size_t uniqueSiblings = 0;
	for (size_t i = 0; i < childrenCount; i++) {
		for (size_t indexParent = 0; indexParent < childrens[i]->myParentCount; indexParent++) {
			if (childrens[i]->myParents[indexParent]->myPersonId == aId) {
				continue;
			}
			int isUnique = 1;
			for (size_t j = 0; j < uniqueSiblings; j++) {
				if (childrens[i]->myParents[indexParent]->myPersonId == aOutSiblings[j]) {
					isUnique = 0;
					break;
				}
			}
			if (isUnique != 0) {
				aOutSiblings[uniqueSiblings] = childrens[i]->myParents[indexParent]->myPersonId;
				uniqueSiblings++;
			}
		}
	}
	return uniqueSiblings;
}

size_t
GetParentsCount(MetaData aMetaData, PersonId aId, ITrace* aTrace) {
	size_t parentCount = 0;
	PersonMeta** parents = NULL;
	for (size_t i = 0; i < aMetaData.myPersonCount; i++) {
		if (aMetaData.myPersons[i].myPersonId == aId) {
			return aMetaData.myPersons[i].myParentCount;
		}
	}
	return 0;
}

void
GetParents(MetaData aMetaData, PersonId aId, PersonId* aOutParents, ITrace* aTrace) {
	size_t parentCount = 0;
	PersonMeta** parents = NULL;
	for (size_t i = 0; i < aMetaData.myPersonCount; i++) {
		if (aMetaData.myPersons[i].myPersonId == aId) {
			parentCount = aMetaData.myPersons[i].myParentCount;
			parents = aMetaData.myPersons[i].myParents;
			break;
		}
	}
	if (parents == NULL) {
		return;
	}
	for (size_t i = 0; i < parentCount; i++) {
		aOutParents[i] = parents[i]->myPersonId;
	}
}

size_t
IntersectParents(
	MetaData aMetaData,
	PersonId aId,
	size_t aParentCount,
	PersonId* aInOutParents,
	ITrace* aTrace) {
	size_t parentCount = 0;
	PersonMeta** parents = NULL;
	for (size_t i = 0; i < aMetaData.myPersonCount; i++) {
		if (aMetaData.myPersons[i].myPersonId == aId) {
			parentCount = aMetaData.myPersons[i].myParentCount;
			parents = aMetaData.myPersons[i].myParents;
			break;
		}
	}
	if (parents == NULL) {
		return 0;
	}
	for (size_t i = 0; i < aParentCount; i++) {
		int found = 0;
		for (size_t j = 0; j < parentCount; j++) {
			if (aInOutParents[i] == parents[j]->myPersonId) {
				found = 1;
				break;
			}
		}
		if (found != 0) {
			continue;
		}
		aParentCount--;
		aInOutParents[i] = aInOutParents[aParentCount];
		// NOTE: the moved item needs to be checked as well.
		i--;
	}
	return aParentCount;
}

size_t
GetChildrenCount(MetaData aMetaData, PersonId aId, ITrace* aTrace) {
	for (size_t i = 0; i < aMetaData.myPersonCount; i++) {
		if (aMetaData.myPersons[i].myPersonId == aId) {
			return aMetaData.myPersons[i].myChildCount;
		}
	}
	return 0;
}

void
GetChildrens(MetaData aMetaData, PersonId aId, PersonId* aOutChildrens, ITrace* aTrace) {
	size_t childrenCount = 0;
	PersonMeta** childrens = NULL;
	for (size_t i = 0; i < aMetaData.myPersonCount; i++) {
		if (aMetaData.myPersons[i].myPersonId == aId) {
			childrenCount = aMetaData.myPersons[i].myChildCount;
			childrens = aMetaData.myPersons[i].myChilds;
			break;
		}
	}
	if (childrens == NULL) {
		return;
	}
	for (size_t i = 0; i < childrenCount; i++) {
		aOutChildrens[i] = childrens[i]->myPersonId;
	}
}

size_t
IntersectChildrens(
	MetaData aMetaData,
	PersonId aId,
	size_t aChildrenCount,
	PersonId* aInOutChildrens,
	ITrace* aTrace) {
	size_t childrenCount = 0;
	PersonMeta** childrens = NULL;
	for (size_t i = 0; i < aMetaData.myPersonCount; i++) {
		if (aMetaData.myPersons[i].myPersonId == aId) {
			childrenCount = aMetaData.myPersons[i].myChildCount;
			childrens = aMetaData.myPersons[i].myChilds;
			break;
		}
	}
	if (childrens == NULL) {
		return 0;
	}
	for (size_t i = 0; i < aChildrenCount; i++) {
		int found = 0;
		for (size_t j = 0; j < childrenCount; j++) {
			if (aInOutChildrens[i] == childrens[j]->myPersonId) {
				found = 1;
				break;
			}
		}
		if (found != 0) {
			continue;
		}
		aChildrenCount--;
		aInOutChildrens[i] = aInOutChildrens[aChildrenCount];
		// NOTE: the moved item needs to be checked as well.
		i--;
	}
	return aChildrenCount;
}