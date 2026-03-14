#include "csv_relation_tests.hpp"

#include "auto_free.hpp"
#include "check.hpp"
#include "mock_platform.hpp"

#include <internal_types.h>

#include <array>

bool
Equal(const Relation& aLhs, const Relation& aRhs) {
	return (aLhs.id1 == aRhs.id1 && aLhs.id2 == aRhs.id2);
}

template <class T>
std::basic_ostream<char, T>&
operator<<(std::basic_ostream<char, T>& aOstr, const Relation& aVal) {
	aOstr << '(' << aVal.id1 << '|' << aVal.id2 << ')';
	return aOstr;
}

bool
CSVRelationsCanHandleEmptyFile() {
	MockPlatform platform{};
	AutoFree trace = CreateNoOpTrace();
	AutoFree rel = CreateCSVRelation("", platform, trace);

	auto size = rel->GetAllIdsCount(rel, trace);

	CHECK(size, 0); // NOLINT(readability-simplify-boolean-expr)
	return true;
}

bool
CSVRelationsCanComputeCountOfAllUniqueIds() {
	MockPlatform platform{};
	platform.myFiles.emplace("/relations.csv", R"csv(personA,personB,type,relationship,start,end
1,2,StrictlyLower
1,3,StrictlyLower
2,3,Unrestricted
3,4,StrictlyLower
4,5,StrictlyLower)csv");

	AutoFree trace = CreateNoOpTrace();
	AutoFree rel = CreateCSVRelation("", platform, trace);

	auto size = rel->GetAllIdsCount(rel, trace);

	CHECK(size, 5); // NOLINT(readability-simplify-boolean-expr)
	return true;
}

bool
CSVRelationsCanComputeAllUniqueIds() {
	MockPlatform platform{};
	platform.myFiles.emplace("/relations.csv", R"csv(personA,personB,type,relationship,start,end
5,8,StrictlyLower
5,9,StrictlyLower
8,9,Unrestricted
9,13,StrictlyLower
13,32,StrictlyLower)csv");

	AutoFree trace = CreateNoOpTrace();
	AutoFree rel = CreateCSVRelation("", platform, trace);

	constexpr auto arraysize = 5;
	std::array<PersonId, arraysize> ids{};
	rel->GetAllIds(rel, ids.data(), trace);

	CHECK(ids[0], 5, 32, 13, 8, 9);
	CHECK(ids[1], 5, 32, 13, 8, 9);
	CHECK(ids[2], 5, 32, 13, 8, 9);
	CHECK(ids[3], 5, 32, 13, 8, 9);
	CHECK(ids[4], 5, 32, 13, 8, 9);
	CHECK_EXCLUDE(ids[0], ids[1], ids[2], ids[3], ids[4]);
	CHECK_EXCLUDE(ids[1], ids[2], ids[3], ids[4], ids[0]);
	CHECK_EXCLUDE(ids[2], ids[3], ids[4], ids[0], ids[1]);
	CHECK_EXCLUDE(ids[3], ids[4], ids[0], ids[1], ids[2]);
	CHECK_EXCLUDE(ids[4], ids[0], ids[1], ids[2], ids[3]);
	return true;
}

bool
CSVRelationsCanComputeRelationsCount() {
	MockPlatform platform{};
	platform.myFiles.emplace("/relations.csv", R"csv(personA,personB,type,relationship,start,end
5,8,StrictlyLower
5,9,StrictlyLower
8,9,Unrestricted
9,13,StrictlyLower
13,32,StrictlyLower)csv");

	AutoFree trace = CreateNoOpTrace();
	AutoFree rel = CreateCSVRelation("", platform, trace);

	auto size = rel->GetAllRelationsOfIdCount(rel, 13, trace); // NOLINT

	CHECK(size, 2);
	return true;
}

bool
CSVRelationsCanComputeRelationsOfPerson() {
	MockPlatform platform{};
	platform.myFiles.emplace("/relations.csv", R"csv(personA,personB,type,relationship,start,end
5,8,StrictlyLower
5,9,StrictlyLower
8,9,Unrestricted
9,13,StrictlyLower
13,32,StrictlyLower)csv");

	AutoFree trace = CreateNoOpTrace();
	AutoFree rel = CreateCSVRelation("", platform, trace);

	std::array<Relation, 2> relations{};
	rel->GetAllRelationsOfId(rel, 13, relations.data(), trace); // NOLINT

	CHECK(13, relations[0].id1, relations[0].id2);
	CHECK(13, relations[1].id1, relations[1].id2);
	CHECK_EXCLUDE(relations[0], relations[1]);
	return true;
}
