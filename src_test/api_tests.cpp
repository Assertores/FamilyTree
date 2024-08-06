#include "api_tests.hpp"

#include "auto_free.hpp"
#include "check.hpp"
#include "mock_data_provider.hpp"
#include "mock_platform.hpp"

#include <family_tree/api.h>

#include <array>

bool
CanRetreavePersonThroughAPI() {
	AutoFree context = Create(nullptr);
	MockPlatform platform{};

	AddDataProvider(
		context,
		CreateDataProvider(
			context,
			CreateCSVRelations(context, "", platform, nullptr),
			CreateJSONPersonals(context, "", platform, nullptr),
			nullptr),
		nullptr);

	Person person = GetPerson(context, 0, nullptr);
	CHECK(person.title, "Prof. Dr.");
	CHECK(person.firstNameCount, 3);
	CHECK(person.firstNames[0], "Philippa");
	CHECK(person.firstNames[1], "Rosa");
	CHECK(person.firstNames[2], "Polly");
	CHECK(person.titleOfNobility, "von und zu");
	CHECK(person.lastNameCount, 3);
	CHECK(person.lastNames[0], "Perry");
	CHECK(person.lastNames[1], "Figueroa");
	CHECK(person.lastNames[2], "Russell");
	CHECK(person.gender, "male");
	CHECK(person.dateOfBirth, "04.05.1996");
	CHECK(person.placeOfBirth, "PlaceA");
	CHECK(person.dateOfDeath, "11.11.2021");
	CHECK(person.placeOfDeath, "PlaceB");
	CHECK(person.remark, "RemarkA");
	return true;
}

bool
CanPlayMusicThroughAPI() {
	AutoFree context = Create(nullptr);
	MockPlatform platform{};
	int audioLoadedCount = 0;
	platform.myAudioDeprecates["/abc/Sunday_plans.mp3"] = [&]() { audioLoadedCount++; };

	AddDataProvider(
		context,
		CreateDataProvider(
			context,
			CreateCSVRelations(context, "", platform, nullptr),
			CreateJSONPersonals(context, "", platform, nullptr),
			nullptr),
		nullptr);

	PlayPerson(context, 0, nullptr);

	return !platform.myUnexpectedAudio && audioLoadedCount == 1;
}

bool
CanShowImagesThroughAPI() {
	AutoFree context = Create(nullptr);
	MockPlatform platform{};
	int firstImageLoadedCount = 0;
	int secondImageLoadedCount = 0;
	platform.myImageDeprecates["/abc/Trohnsaal.png"] = [&]() { firstImageLoadedCount++; };
	platform.myImageDeprecates["/abc/Bauhaus.png"] = [&]() { secondImageLoadedCount++; };

	AddDataProvider(
		context,
		CreateDataProvider(
			context,
			CreateCSVRelations(context, "", platform, nullptr),
			CreateJSONPersonals(context, "", platform, nullptr),
			nullptr),
		nullptr);

	ShowImagesOfPerson(context, 0, nullptr);

	return !platform.myUnexpectedImage && firstImageLoadedCount == 1 && secondImageLoadedCount == 1;
}

bool
CanRetreaveRelationsOfPerson() {
	AutoFree context = Create(nullptr);
	CsvPlatform platform{};

	AddDataProvider(
		context,
		CreateDataProvider(
			context,
			CreateCSVRelations(context, "", platform, nullptr),
			CreateJSONPersonals(context, "", platform, nullptr),
			nullptr),
		nullptr);

	size_t count = 0;
	// NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
	auto* relations = GetPersonRelations(context, 0, &count, nullptr);

	return true;
}

bool
CanRetreavePartners() {
	AutoFree context = Create(nullptr);
	MockDataProvider data{};

	AddDataProvider(context, data, nullptr);

	size_t count = 0;
	auto* partners = GetPartners(context, 2, &count, nullptr);

	CHECK(count, 1);
	CHECK(partners[0], 3);

	return true;
}

bool
CanRetreaveSiblings() {
	AutoFree context = Create(nullptr);
	MockDataProvider data{};

	AddDataProvider(context, data, nullptr);

	size_t count = 0;
	auto* siblings = GetSiblings(context, 2, &count, nullptr);

	CHECK(count, 1);
	CHECK(siblings[0], 3);

	return true;
}

bool
CanRetreaveCommonParents() {
	AutoFree context = Create(nullptr);
	MockDataProvider data{};

	AddDataProvider(context, data, nullptr);

	std::array<PersonId, 2> parents{0, 1};

	size_t count = 0;
	auto* childrens = GetCommonChildren(context, parents.size(), parents.data(), &count, nullptr);

	CHECK(count, 3);
	CHECK(2, childrens[0], childrens[1], childrens[2]);
	CHECK(4, childrens[0], childrens[1], childrens[2]);
	CHECK(11, childrens[0], childrens[1], childrens[2]);

	return true;
}

bool
CanRetreaveCommonChildren() {
	AutoFree context = Create(nullptr);
	MockDataProvider data{};

	AddDataProvider(context, data, nullptr);

	std::array<PersonId, 2> childrens{11, 4}; // NOLINT

	size_t count = 0;
	auto* parents = GetCommonParents(context, childrens.size(), childrens.data(), &count, nullptr);

	CHECK(count, 2);
	CHECK(0, parents[0], parents[1]);
	CHECK(1, parents[0], parents[1]);

	return true;
}
