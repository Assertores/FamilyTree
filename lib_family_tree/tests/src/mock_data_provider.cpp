#include "mock_data_provider.hpp"

// NOLINTBEGIN
IDataProvider*
AbstractDataProvider_Copy(IDataProvider* aThis, ITrace* aTrace) {
	return aThis;
}

Person
AbstractDataProvider_GetPerson(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	return ((CDataProvider*)aThis)->myThis->GetPerson(aId);
}

void
AbstractDataProvider_PlayPerson(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	((CDataProvider*)aThis)->myThis->PlayPerson(aId);
}

void
AbstractDataProvider_ShowImages(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	((CDataProvider*)aThis)->myThis->ShowImages(aId);
}

size_t
AbstractDataProvider_GetAllIdsCount(IDataProvider* aThis, ITrace* aTrace) {
	return ((CDataProvider*)aThis)->myThis->GetAllIdsCount();
}

void
AbstractDataProvider_GetAllIds(IDataProvider* aThis, PersonId* aOutId, ITrace* aTrace) {
	((CDataProvider*)aThis)->myThis->GetAllIds(aOutId);
}

size_t
AbstractDataProvider_GetAllRelationsOfIdCount(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	return ((CDataProvider*)aThis)->myThis->GetAllRelationsOfIdCount(aId);
}

void
AbstractDataProvider_GetAllRelationsOfId(
	IDataProvider* aThis,
	PersonId aId,
	Relation* aOutRelation,
	ITrace* aTrace) {
	((CDataProvider*)aThis)->myThis->GetAllRelationsOfId(aId, aOutRelation);
}

RelationType
AbstractDataProvider_GetRelationType(IDataProvider* aThis, Relation aRelation, ITrace* aTrace) {
	return ((CDataProvider*)aThis)->myThis->GetRelationType(aRelation);
}

void
AbstractDataProvider_Free(IDataProvider* aThis, ITrace* aTrace) {
	// TODO: figure this out
}

MockDataProvider::MockDataProvider() {
	myInterface.myInterface.Copy = AbstractDataProvider_Copy;
	myInterface.myInterface.GetPerson = AbstractDataProvider_GetPerson;
	myInterface.myInterface.PlayPerson = AbstractDataProvider_PlayPerson;
	myInterface.myInterface.ShowImages = AbstractDataProvider_ShowImages;
	myInterface.myInterface.GetAllIdsCount = AbstractDataProvider_GetAllIdsCount;
	myInterface.myInterface.GetAllIds = AbstractDataProvider_GetAllIds;
	myInterface.myInterface.GetAllRelationsOfIdCount =
		AbstractDataProvider_GetAllRelationsOfIdCount;
	myInterface.myInterface.GetAllRelationsOfId = AbstractDataProvider_GetAllRelationsOfId;
	myInterface.myInterface.GetRelationType = AbstractDataProvider_GetRelationType;
	myInterface.myInterface.Free = AbstractDataProvider_Free;
	myInterface.myThis = this;
}
// NOLINTEND

MockDataProvider::operator IDataProvider*() { return &myInterface.myInterface; }

// ┌───┐┌───┐
// │ 1 ││ 2 │
// └─┬─┘└─┬─┘
//   ├──┬─┴─────┐
// ┌─┴─┐│┌───┐┌─┴─┐┌───┐
// │ 3 │││ 4 ││ 5 ││ 6 │
// └─┬─┘│└─┬─┘└─┬─┘└─┬─┘
//   └──│┬─┘    │    │
//   ┌──││──────┴────┘
//   │  ││
// ┌─┴─┐││┌───┐
// │ 7 ││││ 8 │
// └─┬─┘││└─┬─┘
//   ├──││──┘
//   │  │└─┐
//   │  │  │
// ┌─┴─┐│┌─┴─┐
// │ 9 │││10 │
// └─┬─┘│└─┬─┘
//   ├──│──┘
//   │  └─┐
// ┌─┴─┐┌─┴─┐┌───┐
// │11 ││12 ││13 │
// └─┬─┘└─┬─┘└─┬─┘
//   └──┬─┤    │
//   ┌──│─┴────┘
// ┌─┴─┐│┌───┐
// │14 │││15 │
// └─┬─┘│└─┬─┘
//   ├──│──┘
//   │  └─┐
// ┌─┴─┐┌─┴─┐
// │16 ││17 │
// └─┬─┘└─┬─┘
//   ├────┘
// ┌─┴─┐
// │18 │
// └───┘
MockDataProvider
MockDataProvider::CreateComplex() {
	MockDataProvider result;

	// NOLINTBEGIN
	Person person{};
	person.id = 1;
	person.remark = "Remark 1";
	result.myPersons[person.id] = person;
	person.id = 2;
	person.remark = "Remark 2";
	result.myPersons[person.id] = person;
	person.id = 3;
	person.remark = "Remark 3";
	result.myPersons[person.id] = person;
	person.id = 4;
	person.remark = "Remark 4";
	result.myPersons[person.id] = person;
	person.id = 5;
	person.remark = "Remark 5";
	result.myPersons[person.id] = person;
	person.id = 6;
	person.remark = "Remark 6";
	result.myPersons[person.id] = person;
	person.id = 7;
	person.remark = "Remark 7";
	result.myPersons[person.id] = person;
	person.id = 8;
	person.remark = "Remark 8";
	result.myPersons[person.id] = person;
	person.id = 9;
	person.remark = "Remark 9";
	result.myPersons[person.id] = person;
	person.id = 10;
	person.remark = "Remark 10";
	result.myPersons[person.id] = person;
	person.id = 11;
	person.remark = "Remark 11";
	result.myPersons[person.id] = person;
	person.id = 12;
	person.remark = "Remark 12";
	result.myPersons[person.id] = person;
	person.id = 13;
	person.remark = "Remark 13";
	result.myPersons[person.id] = person;
	person.id = 14;
	person.remark = "Remark 14";
	result.myPersons[person.id] = person;
	person.id = 15;
	person.remark = "Remark 15";
	result.myPersons[person.id] = person;
	person.id = 16;
	person.remark = "Remark 16";
	result.myPersons[person.id] = person;
	person.id = 17;
	person.remark = "Remark 17";
	result.myPersons[person.id] = person;
	person.id = 18;
	person.remark = "Remark 18";
	result.myPersons[person.id] = person;

	Relation relation{};
	relation.id1 = 1;
	relation.id2 = 3;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 1;
	relation.id2 = 5;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 1;
	relation.id2 = 12;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 2;
	relation.id2 = 3;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 2;
	relation.id2 = 5;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 2;
	relation.id2 = 12;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 3;
	relation.id2 = 10;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 4;
	relation.id2 = 10;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 5;
	relation.id2 = 7;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 6;
	relation.id2 = 7;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 7;
	relation.id2 = 9;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 8;
	relation.id2 = 9;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 9;
	relation.id2 = 11;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 10;
	relation.id2 = 11;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 11;
	relation.id2 = 17;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 12;
	relation.id2 = 17;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 12;
	relation.id2 = 14;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 13;
	relation.id2 = 14;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 14;
	relation.id2 = 16;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 15;
	relation.id2 = 16;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 16;
	relation.id2 = 18;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	relation.id1 = 17;
	relation.id2 = 18;
	result.myRelations[relation.id1].push_back(relation);
	result.myRelations[relation.id2].push_back(relation);
	result.myChildRelations.push_back(relation);
	// NOLINTEND

	return result;
}

Person
MockDataProvider::GetPerson(PersonId aId) const {
	return myPersons.at(aId);
}

void
MockDataProvider::PlayPerson(PersonId aId) {
	myHasPlayed.insert(aId);
}

void
MockDataProvider::ShowImages(PersonId aId) {
	myHasShowImage.insert(aId);
}

size_t
MockDataProvider::GetAllIdsCount() const {
	return myPersons.size();
}

void
MockDataProvider::GetAllIds(PersonId* aOutId) const {
	size_t index = 0;
	for (const auto& person : myPersons) {
		aOutId[index] = person.first;
		index++;
	}
}

size_t
MockDataProvider::GetAllRelationsOfIdCount(PersonId aId) const {
	return myRelations.at(aId).size();
}

void
MockDataProvider::GetAllRelationsOfId(PersonId aId, Relation* aOutRelation) const {
	const auto& relations = myRelations.at(aId);
	for (size_t i = 0; i < relations.size(); i++) {
		aOutRelation[i] = relations[i];
	}
}

RelationType
MockDataProvider::GetRelationType(Relation aRelation) const {
	for (const auto& relation : myChildRelations) {
		if (relation.id1 == aRelation.id1 && relation.id2 == aRelation.id2) {
			return RelationType_StrictlyLower;
		}
		if (relation.id1 == aRelation.id2 && relation.id2 == aRelation.id1) {
			return RelationType_StrictlyHigher;
		}
	}
	return RelationType_Unrestricted;
}
