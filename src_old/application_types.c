#include "application_types.h"

#include "generic_types.h"

void
FreeRelation(Relation aRelation) {}
SetImpl(Relation, EqualRelation, FreeRelation);
SetImpl(Person, EqualPerson, FreePerson);
ArrayRefImpl(ArrPerson);
ArrayImpl(String, FreeString);

Bool
EqualRelation(const Relation* aLhs, const Relation* aRhs) {
	return aLhs->person1 == aRhs->person1 && aLhs->person2 == aRhs->person2 ? True : Different;
}

Bool
EqualPerson(const Person* aLhs, const Person* aRhs) {
	return aLhs->person == aRhs->person ? True : Different;
}

void
MergePersonalData(Person* aPerson, Person aData, Bool aPrefereData) {
	if (aData.person != 0) {
		if (aPrefereData == True || aPerson->person == 0) {
			aPerson->person = aData.person;
		}
	}
	if (aData.title.length != 0) {
		if (aPrefereData == True || aPerson->title.length == 0) {
			aPerson->title = aData.title;
			aData.title.length = 0;
		}
	}
	if (aData.firstNames.length != 0) {
		AppendStrings(&aPerson->firstNames, aData.firstNames);
		aData.firstNames.length = 0;
	}
	if (aData.titleOfNobility.length != 0) {
		if (aPrefereData == True || aPerson->titleOfNobility.length == 0) {
			aPerson->titleOfNobility = aData.titleOfNobility;
			aData.titleOfNobility.length = 0;
		}
	}
	if (aData.lastNames.length != 0) {
		AppendStrings(&aPerson->lastNames, aData.lastNames);
		aData.lastNames.length = 0;
	}
	if (aData.gender.length != 0) {
		if (aPrefereData == True || aPerson->gender.length == 0) {
			aPerson->gender = aData.gender;
			aData.gender.length = 0;
		}
	}
	if (aData.birthDay.length != 0) {
		if (aPrefereData == True || aPerson->birthDay.length == 0) {
			aPerson->birthDay = aData.birthDay;
			aData.birthDay.length = 0;
		}
	}
	if (aData.birthPlace.length != 0) {
		if (aPrefereData == True || aPerson->birthPlace.length == 0) {
			aPerson->birthPlace = aData.birthPlace;
			aData.birthPlace.length = 0;
		}
	}
	if (aData.deathDay.length != 0) {
		if (aPrefereData == True || aPerson->deathDay.length == 0) {
			aPerson->deathDay = aData.deathDay;
			aData.deathDay.length = 0;
		}
	}
	if (aData.deathPlace.length != 0) {
		if (aPrefereData == True || aPerson->deathPlace.length == 0) {
			aPerson->deathPlace = aData.deathPlace;
			aData.deathPlace.length = 0;
		}
	}
	if (aData.remark.length != 0) {
		if (aPrefereData == True || aPerson->remark.length == 0) {
			aPerson->remark = aData.remark;
			aData.remark.length = 0;
		}
	}
	if (aData.audioPath.string.length != 0) {
		if (aPrefereData == True || aPerson->audioPath.string.length == 0) {
			aPerson->audioPath = aData.audioPath;
			Path p = {0};
			aData.audioPath = p;
		}
	}
	if (aData.imagePaths.length != 0) {
		AppendPaths(&aPerson->imagePaths, aData.imagePaths);
		aData.imagePaths.length = 0;
	}

	FreePerson(aData);
}

void
FreePerson(Person aPerson) {
	FreeString(aPerson.title);
	FreeStrings(aPerson.firstNames);
	FreeString(aPerson.titleOfNobility);
	FreeStrings(aPerson.lastNames);
	FreeString(aPerson.gender);
	FreeString(aPerson.birthDay);
	FreeString(aPerson.birthPlace);
	FreeString(aPerson.deathDay);
	FreeString(aPerson.deathPlace);
	FreeString(aPerson.remark);
}

void
FreeGraph(Graph aGraph) {
	FreePersons(aGraph.persons);
	FreeRelations(aGraph.relations);
}
