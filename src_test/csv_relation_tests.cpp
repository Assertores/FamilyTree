#include "csv_relation_tests.hpp"

#include "auto_free.hpp"
#include "check.hpp"
#include "mock_platform.hpp"

#include <internal_types.h>

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
	AbstractPlatform platform{};
	AutoFree rel = CreateCSVRelation("", platform);

	auto size = rel->GetAllIdsCount(rel);

	CHECK(size, 0);
	return true;
}

bool
CSVRelationsCanComputeCountOfAllUniqueIds() {
	CsvPlatform platform{};
	AutoFree rel = CreateCSVRelation("", platform);

	auto size = rel->GetAllIdsCount(rel);

	CHECK(size, 5);
	return true;
}

bool
CSVRelationsCanComputeAllUniqueIds() {
	CsvPlatform platform{};
	AutoFree rel = CreateCSVRelation("", platform);

	PersonId ids[5];
	rel->GetAllIds(rel, ids);

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
	CsvPlatform platform{};
	AutoFree rel = CreateCSVRelation("", platform);

	auto size = rel->GetAllRelationsOfIdCount(rel, 13);

	CHECK(size, 2);
	return true;
}

bool
CSVRelationsCanComputeRelationsOfPerson() {
	CsvPlatform platform{};
	AutoFree rel = CreateCSVRelation("", platform);

	Relation relations[2];
	rel->GetAllRelationsOfId(rel, 13, relations);

	CHECK(13, relations[0].id1, relations[0].id2);
	CHECK(13, relations[1].id1, relations[1].id2);
	CHECK_EXCLUDE(relations[0], relations[1]);
	return true;
}
