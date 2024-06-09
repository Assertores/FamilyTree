#include "auto_free.hpp"
#include "check.hpp"
#include <internal_types.h>
#include "mock_platform.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool
JSONPersonIsZeroIfNoFoldersExist() {
	AbstractPlatform platform{};
	AutoFree person = CreateJSONPerson("", platform);

	auto size = person->GetAllIdsCount(person);

	CHECK(size, 0);
	return true;
}

bool
JSONPersonIsAmountOfFolders() {
	MockPlatform platform{};
	AutoFree person = CreateJSONPerson("", platform);

	auto size = person->GetAllIdsCount(person);

	CHECK(size, 2);
	return true;
}

bool
CanRetreavePersonIds() {
	MockPlatform platform{};
	AutoFree person = CreateJSONPerson("", platform);

	PersonId ids[2];
	person->GetAllIds(person, ids);

	CHECK(ids[0], 5);
	CHECK(ids[1], 32);
	return true;
}

bool
PersonDataCanBeRetreaved() {
	MockPlatform platform{};
	AutoFree person = CreateJSONPerson("", platform);

	Person result;
	person->GetPerson(person, 5, &result);

	CHECK(result.title, "Prof. Dr.");
	CHECK(result.firstNameCount, 3);
	CHECK(result.firstNames[0], "Philippa");
	CHECK(result.firstNames[1], "Rosa");
	CHECK(result.firstNames[2], "Polly");
	CHECK(result.titleOfNobility, "von und zu");
	CHECK(result.lastNameCount, 3);
	CHECK(result.lastNames[0], "Perry");
	CHECK(result.lastNames[1], "Figueroa");
	CHECK(result.lastNames[2], "Russell");
	CHECK(result.gender, "male");
	CHECK(result.dateOfBirth, "04.05.1996");
	CHECK(result.placeOfBirth, "PlaceA");
	CHECK(result.dateOfDeath, "11.11.2021");
	CHECK(result.placeOfDeath, "PlaceB");
	CHECK(result.remark, "RemarkA");
	return true;
}

bool
PlayMusicTriggersCorrectPathOnPlatform() {
	MockPlatform platform{};
	int AudioLoadedCount = 0;
	platform.myAudioDeprecates["/abc/Sunday_plans.mp3"] = [&]() { AudioLoadedCount++; };

	AutoFree person = CreateJSONPerson("", platform);
	person->PlayPerson(person, 5);

	return !platform.myUnexpectedAudio && AudioLoadedCount == 1;
}

bool
OpenImageTriggersCorrectPathOnPlatform() {
	MockPlatform platform{};
	int firstImageLoadedCount = 0;
	int secondImageLoadedCount = 0;
	platform.myImageDeprecates["/abc/Trohnsaal.png"] = [&]() { firstImageLoadedCount++; };
	platform.myImageDeprecates["/abc/Bauhaus.png"] = [&]() { secondImageLoadedCount++; };

	AutoFree person = CreateJSONPerson("", platform);
	person->ShowImages(person, 5);

	return !platform.myUnexpectedImage && firstImageLoadedCount == 1 && secondImageLoadedCount == 1;
}