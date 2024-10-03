#pragma once

#include <family_tree/types.h>

#include <map>
#include <vector>

class AbstractDataProvider;
struct CDataProvider {
	IDataProvider myInterface{};
	AbstractDataProvider* myThis{};
};

class AbstractDataProvider {
public:
	AbstractDataProvider();
	operator IDataProvider*(); // NOLINT(hicpp-explicit-conversions) to seamlessly interact with c
							   // API
	virtual ~AbstractDataProvider() = default;

	virtual Person GetPerson(PersonId aId) { return {}; }
	virtual void PlayPerson(PersonId aId) {}
	virtual void ShowImages(PersonId aId) {}
	virtual size_t GetAllIdsCount() { return 0; }
	virtual void GetAllIds(PersonId* aOutId) {}
	virtual size_t GetAllRelationsOfIdCount(PersonId aId) { return 0; }
	virtual void GetAllRelationsOfId(PersonId aId, Relation* aOutRelation) {}
	virtual RelationType GetRelationType(Relation aRelation) { return RelationType_Unrestricted; }

private:
	CDataProvider myInterface;
};

class MockDataProvider : public AbstractDataProvider {
public:
	MockDataProvider();

	Person GetPerson(PersonId aId) override;
	size_t GetAllIdsCount() override;
	void GetAllIds(PersonId* aOutId) override;
	size_t GetAllRelationsOfIdCount(PersonId aId) override;
	void GetAllRelationsOfId(PersonId aId, Relation* aOutRelation) override;
	RelationType GetRelationType(Relation aRelation) override;

	std::map<PersonId, Person> myPersons;
	std::map<PersonId, std::vector<Relation>> myRelations;
	std::vector<Relation> myChildRelations;
};
