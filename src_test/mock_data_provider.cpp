#include "mock_data_provider.hpp"

IDataProvider*
AbstractDataProvider_Copy(IDataProvider* aThis, ITrace* aTrace) {
	return aThis;
}

Person
AbstractDataProvider_GetPerson(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	return ((C_DataProvider*)aThis)->myThis->GetPerson(aId);
}

void
AbstractDataProvider_PlayPerson(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	((C_DataProvider*)aThis)->myThis->PlayPerson(aId);
}

void
AbstractDataProvider_ShowImages(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	((C_DataProvider*)aThis)->myThis->ShowImages(aId);
}

size_t
AbstractDataProvider_GetAllIdsCount(IDataProvider* aThis, ITrace* aTrace) {
	return ((C_DataProvider*)aThis)->myThis->GetAllIdsCount();
}

void
AbstractDataProvider_GetAllIds(IDataProvider* aThis, PersonId* aOutId, ITrace* aTrace) {
	((C_DataProvider*)aThis)->myThis->GetAllIds(aOutId);
}

size_t
AbstractDataProvider_GetAllRelationsOfIdCount(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	return ((C_DataProvider*)aThis)->myThis->GetAllRelationsOfIdCount(aId);
}

void
AbstractDataProvider_GetAllRelationsOfId(
	IDataProvider* aThis, PersonId aId, Relation* aOutRelation, ITrace* aTrace) {
	((C_DataProvider*)aThis)->myThis->GetAllRelationsOfId(aId, aOutRelation);
}

RelationType
AbstractDataProvider_GetRelationType(IDataProvider* aThis, Relation aRelation, ITrace* aTrace) {
	return ((C_DataProvider*)aThis)->myThis->GetRelationType(aRelation);
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

AbstractDataProvider::operator IDataProvider*() { return &myInterface.myInterface; }

MockDataProvider::MockDataProvider() {
	Person p{};
	p.id = 234;
	p.remark = "Remark 234";
	myPersons[p.id] = p;
	p.id = 34;
	p.remark = "Remark 34";
	myPersons[p.id] = p;
	p.id = 9754;
	p.remark = "Remark 9754";
	myPersons[p.id] = p;
	p.id = 26;
	p.remark = "Remark 26";
	myPersons[p.id] = p;

	Relation r{};
	r.id1 = 234;
	r.id2 = 34;
	myRelations[r.id1].push_back(r);
	myRelations[r.id2].push_back(r);
	myChildRelations.push_back(r);
	r.id1 = 9754;
	r.id2 = 26;
	myRelations[r.id1].push_back(r);
	myRelations[r.id2].push_back(r);
	r.id1 = 234;
	r.id2 = 26;
	myRelations[r.id1].push_back(r);
	myRelations[r.id2].push_back(r);
	myChildRelations.push_back(r);
	r.id1 = 34;
	r.id2 = 9754;
	myRelations[r.id1].push_back(r);
	myRelations[r.id2].push_back(r);
	myChildRelations.push_back(r);
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
	size_t i = 0;
	for (const auto& person : myPersons) {
		aOutId[i] = person.first;
		i++;
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
	for (const auto& it : myChildRelations) {
		if (it.id1 == aRelation.id1 && it.id2 == aRelation.id2) {
			return RelationType_StrictlyLower;
		}
		if (it.id1 == aRelation.id2 && it.id2 == aRelation.id1) {
			return RelationType_StrictlyHigher;
		}
	}
	return RelationType_Unrestricted;
}
