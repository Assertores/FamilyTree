#include "graph.h"

void ResolveGenerationsRecursively(PersonMeta* element, int minGeneration, Bool aFromChild);
void SquashUp(PersonMeta* element);

void
ResolveGenerationsRecursively(PersonMeta* element, int minGeneration, Bool aFromChild) {
	if (element->myGeneration < minGeneration) {
		element->myGeneration = minGeneration;

		for (size_t i = 0; i < element->childs.length; i++) {
			ResolveGenerationsRecursively(
				element->childs.data[i],
				element->myGeneration + 1,
				False);
		}
	}
	if (aFromChild != True) {
		SquashUp(element); // might be permanent loop
	}
}

void
SquashUp(PersonMeta* element) {
	if (element->parents.length == 0) {
		return;
	}

	// alle Parents auf den höhesten wert setzen
	int maxNumber = -1;
	for (size_t i = 0; i < element->parents.length; i++) {
		maxNumber = maxNumber > element->parents.data[i]->myGeneration
						? maxNumber
						: element->parents.data[i]->myGeneration;
	}

	// Childrens von Parents updaten lassen
	for (size_t i = 0; i < element->parents.length; i++) {
		ResolveGenerationsRecursively(element->parents.data[i], maxNumber, True);
	}
}

EFamily
FindFamily(const Familys* aSet, const Family* aKey) {
	for (size_t i = 0; i < aSet->length; i++) {
		if (EqualFamily(&aSet->data[i], aKey) == True) {
			EFamily result = {0};
			result.data = &aSet->data[i];
			result.hasData = True;
			return result;
		}
	}
	EFamily result = {0};
	result.hasData = NotFound;
	return result;
}

ArrPersonMetas
CreateMetaData(Graph aGraph) {
	ArrPersonMetas result = {0};

	for (size_t i = 0; i < aGraph.persons.length; i++) {
		PersonMeta element = {0};
		element.person = aGraph.persons.data + i;
		element.isProxy = False;
		element.myGeneration = -1;
		CalculateBriefWidth(&element);
		AppendArrPersonMeta(&result, element);
	}

	for (size_t i = 0; i < aGraph.relations.length; i++) {
		EPersonMeta p1 = FindTreeElement(&result, aGraph.relations.data[i].person1);
		EPersonMeta p2 = FindTreeElement(&result, aGraph.relations.data[i].person2);
		if (p1.hasData != True || p2.hasData != True) {
			continue;
		}
		AppendPersonMeta(&p1.data->childs, p2.data);
		AppendPersonMeta(&p2.data->parents, p1.data);
	}

	return result;
}

void
ResolveGraph(ArrPersonMetas* aGraph) {
	for (size_t i = 0; i < aGraph->length; i++) {
		if (aGraph->data[i].parents.length == 0) {
			ResolveGenerationsRecursively(&aGraph->data[i], 0, False);
		}
	}
}

Generations
CreateGenerations(ArrPersonMetas* aMeta) {
	int max = 0;
	for (size_t i = 0; i < aMeta->length; i++) {
		max = aMeta->data[i].myGeneration > max ? aMeta->data[i].myGeneration : max;
	}

	Generations result = {0};
	for (int i = 0; i <= max; i++) {
		Generation element = {0};
		AppendGeneration(&result, element);
	}

	for (size_t i = 0; i < aMeta->length; i++) {
		AppendArrRefPersonMeta(&result.data[aMeta->data[i].myGeneration], &aMeta->data[i]);
	}
	return result;
}

void
AddProxys(const Generation* aTopGeneration, Generation* aBottomGeneration) {
	for (size_t i = 0; i < aTopGeneration->length; i++) {
		for (size_t j = 0; j < aTopGeneration->data[i]->childs.length; j++) {
			PersonMeta* original = aTopGeneration->data[i]->childs.data[j];
			EPersonMeta check = FindPersonMeta(aBottomGeneration, original->person->person);
			if (check.hasData == True) {
				continue;
			}

			PersonMeta* proxy = calloc(1, sizeof(PersonMeta)); // TODO: memory leak
			proxy->isProxy = True;
			proxy->person = original->person;
			AppendPersonMeta(&proxy->childs, original);

			if (aTopGeneration->data[i]->isProxy == True) {
				AppendPersonMeta(&proxy->parents, aTopGeneration->data[i]);
			} else {
				proxy->parents = original->parents;
			}

			CalculateBriefWidth(proxy);
			AppendArrRefPersonMeta(aBottomGeneration, proxy); // TODO: dropping errors
		}
	}
}

Familys
CreateFamilies(Generation aTopGeneration, Generation aBottomGeneration) {
	Familys result = {0};
	for (size_t i = 0; i < aBottomGeneration.length; i++) {
		if (aBottomGeneration.data[i]->parents.length == 0) {
			continue;
		}
		EPersonMeta proxy =
			FindPersonMeta(&aTopGeneration, aBottomGeneration.data[i]->person->person);
		if (proxy.hasData == True) {
			Family f = {0};
			AppendPersonMeta(&f.parents, proxy.data);
			AppendPersonMeta(&f.childs, aBottomGeneration.data[i]);
			AppendFamily(&result, f);
			continue;
		}

		Family f = {0};
		for(size_t j = 0; j < aBottomGeneration.data[i]->parents.length; j++){
			AppendPersonMeta(&f.parents, aBottomGeneration.data[i]->parents.data[j]);
		}
		//f.parents = aBottomGeneration.data[i]->parents;

		EFamily family = FindFamily(&result, &f);
		if (family.hasData == True) {
			AppendPersonMeta(&family.data->childs, aBottomGeneration.data[i]);
			continue;
		}
		AppendPersonMeta(&f.childs, aBottomGeneration.data[i]);
		AppendFamily(&result, f);
	}
	return result;
}

void
CalculateBriefWidth(PersonMeta* aPerson) {
	if (aPerson->isProxy == True) {
		aPerson->briefWidth = 1;
		return;
	}

	int nameWidth = aPerson->person->title.length + 1;
	nameWidth += aPerson->person->firstNames.data[0].length + 1;
	for (size_t i = 1; i < aPerson->person->firstNames.length; i++) {
		nameWidth += 3;
	}
	nameWidth += aPerson->person->titleOfNobility.length + 1;
	nameWidth += aPerson->person->lastNames.data[0].length;

	int infoWidth = 1 + 3 + 2;
	infoWidth += aPerson->person->birthDay.length;
	infoWidth += 3 + 2;
	infoWidth += aPerson->person->deathDay.length;

	aPerson->briefWidth = 2 + (nameWidth > infoWidth ? nameWidth : infoWidth);
	if (aPerson->briefWidth % 2 == 0) {
		aPerson->briefWidth++;
	}
}
