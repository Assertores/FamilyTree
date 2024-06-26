#include "api_tests.hpp"

#include "auto_free.hpp"
#include "check.hpp"
#include "mock_platform.hpp"

#include <family_tree/api.h>

bool
CanRetreavePersonThroughAPI() {
	AutoFree context = Create(NULL);
	MockPlatform platform{};

	AddDataProvider(
		context,
		CreateDataProvider(
			context,
			CreateCSVRelations(context, "", platform, NULL),
			CreateJSONPersonals(context, "", platform, NULL),
			NULL),
		NULL);

	Person person = GetPerson(context, 0, NULL);
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
	AutoFree context = Create(NULL);
	MockPlatform platform{};
	int AudioLoadedCount = 0;
	platform.myAudioDeprecates["/abc/Sunday_plans.mp3"] = [&]() { AudioLoadedCount++; };

	AddDataProvider(
		context,
		CreateDataProvider(
			context,
			CreateCSVRelations(context, "", platform, NULL),
			CreateJSONPersonals(context, "", platform, NULL),
			NULL),
		NULL);

	PlayPerson(context, 0, NULL);

	return !platform.myUnexpectedAudio && AudioLoadedCount == 1;
}

bool
CanShowImagesThroughAPI() {
	AutoFree context = Create(NULL);
	MockPlatform platform{};
	int firstImageLoadedCount = 0;
	int secondImageLoadedCount = 0;
	platform.myImageDeprecates["/abc/Trohnsaal.png"] = [&]() { firstImageLoadedCount++; };
	platform.myImageDeprecates["/abc/Bauhaus.png"] = [&]() { secondImageLoadedCount++; };

	AddDataProvider(
		context,
		CreateDataProvider(
			context,
			CreateCSVRelations(context, "", platform, NULL),
			CreateJSONPersonals(context, "", platform, NULL),
			NULL),
		NULL);

	ShowImagesOfPerson(context, 0, NULL);

	return !platform.myUnexpectedImage && firstImageLoadedCount == 1 && secondImageLoadedCount == 1;
}

bool
CanRetreaveRelationsOfPerson() {
	AutoFree context = Create(NULL);
	CsvPlatform platform{};

	AddDataProvider(
		context,
		CreateDataProvider(
			context,
			CreateCSVRelations(context, "", platform, NULL),
			CreateJSONPersonals(context, "", platform, NULL),
			NULL),
		NULL);

	size_t count = 0;
	auto relations = GetPersonRelations(context, 0, &count, NULL);

	return true;
}
