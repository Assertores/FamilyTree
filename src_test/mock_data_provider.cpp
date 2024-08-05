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

AbstractDataProvider::AbstractDataProvider() {
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

AbstractDataProvider::operator IDataProvider*() { return &myInterface.myInterface; }

MockDataProvider::MockDataProvider() {
	Person person{};
	person.id = 234; // NOLINT
	person.remark = "Remark 234";
	myPersons[person.id] = person;
	person.id = 34; // NOLINT
	person.remark = "Remark 34";
	myPersons[person.id] = person;
	person.id = 9754; // NOLINT
	person.remark = "Remark 9754";
	myPersons[person.id] = person;
	person.id = 26; // NOLINT
	person.remark = "Remark 26";
	myPersons[person.id] = person;

	Relation relation{};
	relation.id1 = 234; // NOLINT
	relation.id2 = 34;	// NOLINT
	myRelations[relation.id1].push_back(relation);
	myRelations[relation.id2].push_back(relation);
	myChildRelations.push_back(relation);
	relation.id1 = 9754; // NOLINT
	relation.id2 = 26;	 // NOLINT
	myRelations[relation.id1].push_back(relation);
	myRelations[relation.id2].push_back(relation);
	relation.id1 = 234; // NOLINT
	relation.id2 = 26;	// NOLINT
	myRelations[relation.id1].push_back(relation);
	myRelations[relation.id2].push_back(relation);
	myChildRelations.push_back(relation);
	relation.id1 = 34;	 // NOLINT
	relation.id2 = 9754; // NOLINT
	myRelations[relation.id1].push_back(relation);
	myRelations[relation.id2].push_back(relation);
	myChildRelations.push_back(relation);
}

Person
MockDataProvider::GetPerson(PersonId aId) {
	return myPersons[aId];
}

size_t
MockDataProvider::GetAllIdsCount() {
	return myPersons.size();
}

void
MockDataProvider::GetAllIds(PersonId* aOutId) {
	size_t index = 0;
	for (const auto& person : myPersons) {
		aOutId[index] = person.first;
		index++;
	}
}

size_t
MockDataProvider::GetAllRelationsOfIdCount(PersonId aId) {
	return myRelations[aId].size();
}

void
MockDataProvider::GetAllRelationsOfId(PersonId aId, Relation* aOutRelation) {
	auto& relations = myRelations[aId];
	for (size_t i = 0; i < relations.size(); i++) {
		aOutRelation[i] = relations[i];
	}
}

RelationType
MockDataProvider::GetRelationType(Relation aRelation) {
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
