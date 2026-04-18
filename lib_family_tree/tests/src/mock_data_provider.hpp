#pragma once

#include <family_tree/types.h>

#include <map>
#include <vector>
#include <set>

class MockDataProvider;
struct CDataProvider {
	IDataProvider myInterface{};
	MockDataProvider* myThis{};
};

class MockDataProvider {
public:
	MockDataProvider();
	operator IDataProvider*(); // NOLINT(hicpp-explicit-conversions) to seamlessly interact with c API

	static MockDataProvider CreateComplex();

	[[nodiscard]] Person GetPerson(PersonId aId) const;
	void PlayPerson(PersonId aId);
	void ShowImages(PersonId aId);
	[[nodiscard]] size_t GetAllIdsCount() const;
	void GetAllIds(PersonId* aOutId) const;
	[[nodiscard]] size_t GetAllRelationsOfIdCount(PersonId aId) const;
	void GetAllRelationsOfId(PersonId aId, Relation* aOutRelation) const;
	[[nodiscard]] RelationType GetRelationType(Relation aRelation) const;

	// NOLINTBEGIN
	// to ease testing direct access is fine.
	std::map<PersonId, Person> myPersons;
	std::map<PersonId, std::vector<Relation>> myRelations;
	std::vector<Relation> myChildRelations;

	std::set<PersonId> myHasPlayed;
	std::set<PersonId> myHasShowImage;
	// NOLINTEND

private:
	CDataProvider myInterface;
};

