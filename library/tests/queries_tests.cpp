// Unit tests of the Queries: Details of a Person, the search, and the Family Tree.

#include "queries_tests.hpp"

#include "support/test_support.hpp"

#include <check.hpp>
#include <family_tree/api.h>

#include <algorithm>
#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace {

using namespace test;

using Ids = std::vector<FT_PersonId>;

struct Data {
	DataFolder folder;
	ContextGuard context;
	TraceBuffer trace;
	FT_Result result = FT_ERROR_INVALID_ARGUMENT;

	// aPersons maps each Person Id to the further keys of its Person File.
	Data(const std::map<FT_PersonId, std::string>& aPersons, const std::vector<std::string>& aRows)
		: folder(PrivFolder(aPersons, aRows))
		, result(Create(folder, context, trace)) {}

	[[nodiscard]] size_t PersonCount() const {
		size_t count = 0;
		(void)FT_GetPersonCount(context.Get(), &count, nullptr);
		return count;
	}

	[[nodiscard]] size_t FamilyCount() const {
		size_t count = 0;
		(void)FT_GetFamilyCount(context.Get(), &count, nullptr);
		return count;
	}

	[[nodiscard]] Ids Search(const FT_Person& aPrototype, size_t aMinimumMatches) const {
		std::vector<FT_PersonId> found(PersonCount());
		size_t count = 0;
		if (FT_Search(
				context.Get(),
				&aPrototype,
				aMinimumMatches,
				found.data(),
				found.size(),
				&count,
				nullptr)
			!= FT_OK) {
			return {-1};
		}
		return Sorted(found.data(), count);
	}

private:
	static DataFolder PrivFolder(
		const std::map<FT_PersonId, std::string>& aPersons,
		const std::vector<std::string>& aRows) {
		DataFolder folder;
		for (const auto& [personId, keys] : aPersons) {
			folder.AddPerson(std::to_string(personId), PersonFile(personId, keys));
		}
		folder.SetRelations(aRows);
		return folder;
	}
};

// The Family Tree around a Person, with the relative Generation of each Person, and each Family
// as its sorted Parents and Children.
struct Tree {
	FT_Result result = FT_ERROR_INVALID_ARGUMENT;
	std::map<FT_PersonId, int> generations;
	std::vector<std::pair<Ids, Ids>> families;

	Tree(const Data& aData, FT_PersonId aPerson, size_t aDistance) {
		std::vector<FT_TreePerson> persons(aData.PersonCount());
		std::vector<FT_FamilyIndex> indices(aData.FamilyCount());
		size_t personCount = 0;
		size_t familyCount = 0;
		result = FT_GetFamilyTree(
			aData.context.Get(),
			aPerson,
			aDistance,
			persons.data(),
			persons.size(),
			&personCount,
			indices.data(),
			indices.size(),
			&familyCount,
			nullptr);
		if (result != FT_OK) {
			return;
		}
		for (size_t i = 0; i < personCount; ++i) {
			generations[persons[i].person] = persons[i].relativeGeneration;
		}
		for (size_t i = 0; i < familyCount; ++i) {
			const FT_Family* family = nullptr;
			if (FT_GetFamily(aData.context.Get(), indices[i], &family, nullptr) == FT_OK) {
				families.emplace_back(
					Sorted(family->parents, family->parentCount),
					Sorted(family->children, family->childCount));
			}
		}
		std::sort(families.begin(), families.end());
	}

	[[nodiscard]] Ids Persons() const {
		Ids ids;
		for (const auto& [personId, generation] : generations) {
			ids.push_back(personId);
		}
		return ids;
	}
};

FT_Person
EmptyPrototype() {
	return FT_Person{};
}

// docs/requirements/req_0014.md
bool
AnUnknownPersonIdIsAnError() {
	const Data data({{1, ""}}, {});
	CHECK(data.result, FT_OK);
	const FT_Person* person = nullptr;
	CHECK(FT_GetPerson(data.context.Get(), 2, &person, nullptr), FT_ERROR_UNKNOWN_PERSON);
	return true;
}

// docs/requirements/req_0015.md
bool
AMissingDetailIsTheDefaultString() {
	const Data data({{1, R"("FirstNames": ["Anna"])"}}, {});
	const FT_Person* person = nullptr;
	CHECK(FT_GetPerson(data.context.Get(), 1, &person, nullptr), FT_OK);
	CHECK(person->title != nullptr, true);
	CHECK(Text(person->title), "");
	bool missing = false;
	CHECK(FT_IsDefaultString(data.context.Get(), person->title, &missing, nullptr), FT_OK);
	CHECK(missing, true);
	CHECK(
		FT_IsDefaultString(data.context.Get(), person->firstNames.items[0], &missing, nullptr),
		FT_OK);
	CHECK(missing, false);
	return true;
}

// docs/requirements/req_0050.md, docs/requirements/req_0015.md
bool
AnEmptyTextInThePersonFileIsNotMissing() {
	const Data data({{1, R"("Title": "")"}}, {});
	const FT_Person* person = nullptr;
	CHECK(FT_GetPerson(data.context.Get(), 1, &person, nullptr), FT_OK);
	CHECK(Text(person->title), "");
	bool missing = true;
	CHECK(FT_IsDefaultString(data.context.Get(), person->title, &missing, nullptr), FT_OK);
	CHECK(missing, false);
	return true;
}

// docs/requirements/req_0015.md
bool
AMissingListHasNoItemsButASafeFirstItem() {
	const Data data({{1, ""}}, {});
	const FT_Person* person = nullptr;
	CHECK(FT_GetPerson(data.context.Get(), 1, &person, nullptr), FT_OK);
	CHECK(person->firstNames.count, 0U);
	CHECK(person->firstNames.items != nullptr, true);
	bool missing = false;
	CHECK(
		FT_IsDefaultString(data.context.Get(), person->firstNames.items[0], &missing, nullptr),
		FT_OK);
	CHECK(missing, true);
	CHECK(person->placesOfResidence.count, 0U);
	CHECK(person->placesOfResidence.items != nullptr, true);
	missing = false;
	CHECK(
		FT_IsDefaultString(
			data.context.Get(),
			person->placesOfResidence.items[0].name,
			&missing,
			nullptr),
		FT_OK);
	CHECK(missing, true);
	return true;
}

// docs/requirements/req_0050.md, docs/decisions/adr_0020.md
bool
AnEmptyListInThePersonFileIsNotMissing() {
	const Data data({{1, R"("Profession": [], "PlaceOfResidence": [])"}}, {});
	const FT_Person* person = nullptr;
	CHECK(FT_GetPerson(data.context.Get(), 1, &person, nullptr), FT_OK);
	CHECK(person->professions.count, 0U);
	CHECK(person->professions.items != nullptr, true);
	CHECK(Text(person->professions.items[0]), "");
	bool missing = true;
	CHECK(
		FT_IsDefaultString(data.context.Get(), person->professions.items[0], &missing, nullptr),
		FT_OK);
	CHECK(missing, false);
	CHECK(person->placesOfResidence.count, 0U);
	CHECK(person->placesOfResidence.items != nullptr, true);
	missing = true;
	CHECK(
		FT_IsDefaultString(
			data.context.Get(),
			person->placesOfResidence.items[0].name,
			&missing,
			nullptr),
		FT_OK);
	CHECK(missing, false);
	return true;
}

// docs/requirements/req_0016.md
bool
SearchFindsAnyOfSeveralFirstNames() {
	const Data data(
		{{1, R"("FirstNames": ["Philippa", "Rosa"])"}, {2, R"("FirstNames": ["Anna"])"}},
		{});
	const char* rosa[] = {"Rosa"};
	auto prototype = EmptyPrototype();
	prototype.firstNames = {rosa, 1};
	CHECK(data.Search(prototype, 1), Ids({1}));
	return true;
}

// docs/requirements/req_0016.md
bool
SearchFindsPartOfAText() {
	const Data data({{1, R"("LastNames": ["Perry"])"}, {2, R"("LastNames": ["Huber"])"}}, {});
	const char* per[] = {"Per"};
	auto prototype = EmptyPrototype();
	prototype.lastNames = {per, 1};
	CHECK(data.Search(prototype, 1), Ids({1}));
	return true;
}

// docs/requirements/req_0016.md
bool
SearchIgnoresEmptyTexts() {
	const Data data({{1, R"("FirstNames": ["Anna"])"}}, {});
	const char* anna[] = {"Anna"};
	auto prototype = EmptyPrototype();
	prototype.firstNames = {anna, 1};
	prototype.placeOfBirth = "";
	CHECK(data.Search(prototype, 1), Ids({1}));
	return true;
}

// docs/requirements/req_0016.md
bool
AMissingDetailNeverMatches() {
	const Data data({{1, R"("PlaceOfBirth": "Würzburg")"}, {2, ""}}, {});
	auto prototype = EmptyPrototype();
	prototype.placeOfBirth = "a";
	CHECK(data.Search(prototype, 1), Ids());
	prototype.placeOfBirth = "W";
	CHECK(data.Search(prototype, 1), Ids({1}));
	return true;
}

// docs/requirements/req_0016.md
bool
SearchNeedsTheMinimumNumberOfMatches() {
	const Data data(
		{
			{1, R"("FirstNames": ["Anna"], "PlaceOfBirth": "Berlin")"},
			{2, R"("FirstNames": ["Anna"], "PlaceOfBirth": "Hamburg")"},
		},
		{});
	const char* anna[] = {"Anna"};
	auto prototype = EmptyPrototype();
	prototype.firstNames = {anna, 1};
	prototype.placeOfBirth = "Berlin";
	CHECK(data.Search(prototype, 2), Ids({1}));
	CHECK(data.Search(prototype, 1), Ids({1, 2}));
	CHECK(data.Search(prototype, 3), Ids());
	return true;
}

// docs/requirements/req_0016.md
bool
AnEmptySearchWithMinimumZeroFindsEveryPerson() {
	const Data data({{1, ""}, {2, ""}, {5, ""}}, {});
	CHECK(data.Search(EmptyPrototype(), 0), Ids({1, 2, 5}));
	return true;
}

// docs/requirements/req_0016.md, docs/requirements/req_0003.md
bool
SearchMatchesAnyLineOfTheRemarks() {
	const Data data({{1, R"("Remarks": ["erste Zeile", "Bäcker in Würzburg"])"}}, {});
	const char* line[] = {"Bäcker"};
	auto prototype = EmptyPrototype();
	prototype.remarks = {line, 1};
	CHECK(data.Search(prototype, 1), Ids({1}));
	return true;
}

// docs/requirements/req_0016.md
bool
SearchMatchesAPlaceOfResidence() {
	const Data data(
		{{1, R"("PlaceOfResidence": [{"Name": "Bamberg", "StartDate": "1946"}])"}, {2, ""}},
		{});
	const FT_Residence residence{"Bam", nullptr, nullptr};
	auto prototype = EmptyPrototype();
	prototype.placesOfResidence = {&residence, 1};
	CHECK(data.Search(prototype, 1), Ids({1}));
	return true;
}

// docs/decisions/adr_0004.md
bool
SearchFailsWithATooSmallBuffer() {
	const Data data({{1, ""}, {2, ""}}, {});
	FT_PersonId found[1] = {};
	size_t count = 99;
	const auto prototype = EmptyPrototype();
	CHECK(
		FT_Search(data.context.Get(), &prototype, 0, found, 1, &count, nullptr),
		FT_ERROR_BUFFER_TOO_SMALL);
	CHECK(count, 99U);
	return true;
}

// docs/requirements/req_0022.md, docs/features/ftr_0004.md#the-closest-family
bool
FamilyTreeAtDistanceZeroHoldsTheOwnFamilies() {
	// A=1 and B=2 are the Parents of C=3 and D=4; E=5 and F=6 are the Parents of A.
	const Data data(
		{{1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}, {6, ""}},
		{
			"1,3,StrictlyLower",
			"2,3,StrictlyLower",
			"1,4,StrictlyLower",
			"2,4,StrictlyLower",
			"5,1,StrictlyLower",
			"6,1,StrictlyLower",
		});
	Tree tree(data, 1, 0);
	CHECK(tree.result, FT_OK);
	CHECK(tree.Persons(), Ids({1, 2, 3, 4, 5, 6}));
	CHECK(tree.generations[5], -1);
	CHECK(tree.generations[6], -1);
	CHECK(tree.generations[1], 0);
	CHECK(tree.generations[2], 0);
	CHECK(tree.generations[3], 1);
	CHECK(tree.generations[4], 1);
	CHECK(tree.families.size(), 2U);
	return true;
}

// docs/requirements/req_0022.md, docs/features/ftr_0004.md#one-step-further
bool
FamilyTreeOneStepFurtherAddsTheNextFamily() {
	// A=1 and B=2 are the Parents of C=3; C and G=4 are the Parents of H=5.
	const Data data(
		{{1, ""}, {2, ""}, {3, ""}, {4, ""}, {5, ""}},
		{"1,3,StrictlyLower", "2,3,StrictlyLower", "3,5,StrictlyLower", "4,5,StrictlyLower"});
	const Tree near(data, 1, 0);
	CHECK(near.result, FT_OK);
	CHECK(near.Persons(), Ids({1, 2, 3}));
	Tree far(data, 1, 1);
	CHECK(far.result, FT_OK);
	CHECK(far.Persons(), Ids({1, 2, 3, 4, 5}));
	CHECK(far.generations[4], far.generations[3]);
	CHECK(far.families.size(), 2U);
	return true;
}

// docs/requirements/req_0022.md, docs/features/ftr_0004.md#families-stay-whole
bool
FamilyTreeKeepsFamiliesWhole() {
	const Data data({{1, ""}, {2, ""}, {3, ""}}, {"1,3,StrictlyLower", "2,3,StrictlyLower"});
	Tree tree(data, 3, 0);
	CHECK(tree.result, FT_OK);
	CHECK(tree.Persons(), Ids({1, 2, 3}));
	CHECK(tree.families.size(), 1U);
	CHECK(tree.families[0].first, Ids({1, 2}));
	CHECK(tree.families[0].second, Ids({3}));
	return true;
}

// docs/requirements/req_0022.md, docs/features/ftr_0004.md#a-person-without-a-family
bool
FamilyTreeOfAPersonWithoutFamilyHoldsOnlyThePerson() {
	const Data data({{1, ""}, {2, ""}, {7, ""}}, {"1,2,StrictlyLower"});
	Tree tree(data, 7, 3);
	CHECK(tree.result, FT_OK);
	CHECK(tree.Persons(), Ids({7}));
	CHECK(tree.generations[7], 0);
	CHECK(tree.families.size(), 0U);
	return true;
}

// docs/decisions/adr_0009.md
bool
FamilyTreeFailsWithATooSmallBuffer() {
	const Data data({{1, ""}, {2, ""}, {3, ""}}, {"1,3,StrictlyLower", "2,3,StrictlyLower"});
	FT_TreePerson persons[2] = {};
	FT_FamilyIndex families[1] = {};
	size_t personCount = 0;
	size_t familyCount = 0;
	CHECK(
		FT_GetFamilyTree(
			data.context.Get(),
			3,
			0,
			persons,
			2,
			&personCount,
			families,
			1,
			&familyCount,
			nullptr),
		FT_ERROR_BUFFER_TOO_SMALL);
	return true;
}

// docs/requirements/req_0022.md
bool
FamilyTreeOfAnUnknownPersonIsAnError() {
	const Data data({{1, ""}}, {});
	const Tree tree(data, 2, 0);
	CHECK(tree.result, FT_ERROR_UNKNOWN_PERSON);
	return true;
}

} // namespace

bool
QueriesSuite() {
	auto result = SUITE("Queries");
	result &= RUN(AnUnknownPersonIdIsAnError);
	result &= RUN(AMissingDetailIsTheDefaultString);
	result &= RUN(AnEmptyTextInThePersonFileIsNotMissing);
	result &= RUN(AMissingListHasNoItemsButASafeFirstItem);
	result &= RUN(AnEmptyListInThePersonFileIsNotMissing);
	result &= RUN(SearchFindsAnyOfSeveralFirstNames);
	result &= RUN(SearchFindsPartOfAText);
	result &= RUN(SearchIgnoresEmptyTexts);
	result &= RUN(AMissingDetailNeverMatches);
	result &= RUN(SearchNeedsTheMinimumNumberOfMatches);
	result &= RUN(AnEmptySearchWithMinimumZeroFindsEveryPerson);
	result &= RUN(SearchMatchesAnyLineOfTheRemarks);
	result &= RUN(SearchMatchesAPlaceOfResidence);
	result &= RUN(SearchFailsWithATooSmallBuffer);
	result &= RUN(FamilyTreeAtDistanceZeroHoldsTheOwnFamilies);
	result &= RUN(FamilyTreeOneStepFurtherAddsTheNextFamily);
	result &= RUN(FamilyTreeKeepsFamiliesWhole);
	result &= RUN(FamilyTreeOfAPersonWithoutFamilyHoldsOnlyThePerson);
	result &= RUN(FamilyTreeFailsWithATooSmallBuffer);
	result &= RUN(FamilyTreeOfAnUnknownPersonIsAnError);
	return result;
}
