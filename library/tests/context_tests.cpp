// Unit tests of the Context: independence of several Contexts, and releasing everything.

#include "context_tests.hpp"

#include "support/test_support.hpp"

#include <check.hpp>

#include <vector>

namespace {

using namespace test;

DataFolder
Persons(FT_PersonId aFirst, FT_PersonId aLast) {
	DataFolder folder;
	for (FT_PersonId id = aFirst; id <= aLast; ++id) {
		folder.AddPerson(std::to_string(id), PersonFile(id, R"("FirstNames": ["Anna"])"));
	}
	folder.SetRelations({});
	return folder;
}

std::vector<FT_PersonId>
Everyone(const FT_Context* aContext) {
	size_t capacity = 0;
	(void)FT_GetPersonCount(aContext, &capacity, nullptr);
	std::vector<FT_PersonId> found(capacity);
	size_t count = 0;
	FT_Person prototype{};
	if (FT_Search(aContext, &prototype, 0, found.data(), found.size(), &count, nullptr) != FT_OK) {
		return {-1};
	}
	return Sorted(found.data(), count);
}

// docs/requirements/req_0041.md
bool
TwoContextsDoNotInfluenceEachOther() {
	auto firstFolder = Persons(1, 2);
	auto secondFolder = Persons(10, 12);
	ContextGuard first;
	ContextGuard second;
	TraceBuffer trace;
	CHECK(Create(firstFolder, first, trace), FT_OK);
	CHECK(Create(secondFolder, second, trace), FT_OK);
	const auto before = Everyone(second.Get());
	CHECK(Everyone(first.Get()), std::vector<FT_PersonId>({1, 2}));
	CHECK(Everyone(second.Get()), before);
	const FT_Person* person = nullptr;
	CHECK(FT_GetPerson(first.Get(), 10, &person, nullptr), FT_ERROR_UNKNOWN_PERSON);
	return true;
}

// docs/requirements/req_0041.md
bool
FreeingOneContextLeavesTheOtherUsable() {
	auto firstFolder = Persons(1, 2);
	auto secondFolder = Persons(10, 12);
	FT_Context* first = nullptr;
	ContextGuard second;
	CHECK(firstFolder.Create(&first, nullptr), FT_OK);
	CHECK(secondFolder.Create(&second, nullptr), FT_OK);
	const FT_Person* person = nullptr;
	CHECK(FT_GetPerson(second.Get(), 11, &person, nullptr), FT_OK);
	CHECK(FT_Free(first, nullptr), FT_OK);
	CHECK(Text(person->firstNames.items[0]), "Anna");
	CHECK(Everyone(second.Get()), std::vector<FT_PersonId>({10, 11, 12}));
	return true;
}

// docs/requirements/req_0042.md
// The leak check of the test runner (tests/framework/check.hpp) fails the run if anything the
// Library returned here is still allocated after FT_Free.
bool
FreeingAContextReleasesEverythingItReturned() {
	DataFolder folder;
	folder
		.AddPerson(
			"1",
			PersonFile(1, R"("FirstNames": ["Anna"], "Images": ["a.png"], "Audio": "a.mp3")"))
		.AddPerson("2", PersonFile(2))
		.AddPerson("3", PersonFile(3))
		.AddFile("1/a.png", "")
		.AddFile("1/a.mp3", "")
		.SetRelations({"1,3,StrictlyLower", "2,3,StrictlyLower"});
	FT_Context* context = nullptr;
	CHECK(folder.Create(&context, nullptr), FT_OK);
	const FT_Person* person = nullptr;
	const FT_PersonId* ids = nullptr;
	const FT_Relation* relations = nullptr;
	const char* const* pictures = nullptr;
	const char* recording = nullptr;
	size_t count = 0;
	CHECK(FT_GetPerson(context, 1, &person, nullptr), FT_OK);
	CHECK(FT_GetRelations(context, 1, &relations, &count, nullptr), FT_OK);
	CHECK(FT_GetChildren(context, 1, &ids, &count, nullptr), FT_OK);
	CHECK(FT_GetPartners(context, 1, &ids, &count, nullptr), FT_OK);
	CHECK(FT_GetPicturePaths(context, 1, &pictures, &count, nullptr), FT_OK);
	CHECK(FT_GetRecordingPath(context, 1, &recording, nullptr), FT_OK);
	CHECK(Everyone(context).size(), 3U);
	CHECK(FT_Free(context, nullptr), FT_OK);
	return true;
}

// docs/requirements/req_0047.md
// The leak check of the test runner fails the run if the failed creation left memory allocated.
bool
AFailedCreationReleasesEverything() {
	DataFolder folder;
	folder.AddPerson("1", PersonFile(1))
		.AddPerson("2", PersonFile(2))
		.SetRelations({"1,2,StrictlyLower", "2,1,StrictlyLower"});
	FT_Context* context = nullptr;
	CHECK(folder.Create(&context, nullptr), FT_ERROR_CONFLICTING_GENERATIONS);
	CHECK(context == nullptr, true);
	return true;
}

// docs/decisions/adr_0013.md
bool
FreeingNoContextIsAnInvalidArgument() {
	CHECK(FT_Free(nullptr, nullptr), FT_ERROR_INVALID_ARGUMENT);
	return true;
}

// docs/decisions/adr_0013.md
bool
QueriesWithoutContextAreInvalidArguments() {
	size_t count = 0;
	const FT_Person* person = nullptr;
	CHECK(FT_GetPersonCount(nullptr, &count, nullptr), FT_ERROR_INVALID_ARGUMENT);
	CHECK(FT_GetPerson(nullptr, 1, &person, nullptr), FT_ERROR_INVALID_ARGUMENT);
	return true;
}

} // namespace

bool
ContextSuite() {
	auto result = SUITE("Context");
	result &= RUN(TwoContextsDoNotInfluenceEachOther);
	result &= RUN(FreeingOneContextLeavesTheOtherUsable);
	result &= RUN(FreeingAContextReleasesEverythingItReturned);
	result &= RUN(AFailedCreationReleasesEverything);
	result &= RUN(FreeingNoContextIsAnInvalidArgument);
	result &= RUN(QueriesWithoutContextAreInvalidArguments);
	return result;
}
