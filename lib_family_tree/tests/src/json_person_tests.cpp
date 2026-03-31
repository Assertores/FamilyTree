#include "auto_free.hpp"
#include "check.hpp"
#include "mock_platform.hpp"

#include <internal_types.h>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>

bool
JSONPersonIsZeroIfNoFoldersExist() {
	MockPlatform platform{};
	AutoFree trace = CreateNoOpTrace();
	AutoFree person = CreateJSONPerson("", platform, trace);

	auto size = person->GetAllIdsCount(person, trace);

	CHECK(size, 0);
	return true;
}

bool
JSONPersonIsAmountOfFolders() {
	MockPlatform platform{};
	platform.myFolders.emplace("", "a");
	platform.myFolders.emplace("", "b");
	platform.myFiles.emplace(
		"/a/data.json",
		R"json({"Version": "3B5589D2-D9AF-40A8-BC40-574DAB6FFC57","Id": 5})json");
	platform.myFiles.emplace(
		"/b/data.json",
		R"json({"Version": "3B5589D2-D9AF-40A8-BC40-574DAB6FFC57","Id": 32})json");

	AutoFree trace = CreateNoOpTrace();
	AutoFree person = CreateJSONPerson("", platform, trace);

	auto size = person->GetAllIdsCount(person, trace);

	CHECK(size, 2);
	return true;
}

bool
CanRetreavePersonIds() {
	MockPlatform platform{};
	platform.myFolders.emplace("", "a");
	platform.myFolders.emplace("", "b");
	platform.myFiles.emplace(
		"/a/data.json",
		R"json({"Version": "3B5589D2-D9AF-40A8-BC40-574DAB6FFC57","Id": 5})json");
	platform.myFiles.emplace(
		"/b/data.json",
		R"json({"Version": "3B5589D2-D9AF-40A8-BC40-574DAB6FFC57","Id": 32})json");

	AutoFree trace = CreateNoOpTrace();
	AutoFree person = CreateJSONPerson("", platform, trace);

	std::array<PersonId, 2> ids{};
	person->GetAllIds(person, ids.data(), trace);

	CHECK(ids[0], 5);
	CHECK(ids[1], 32);
	return true;
}

bool
PersonDataCanBeRetreaved() {
	MockPlatform platform{};
	platform.myFolders.emplace("", "a");
	platform.myFiles.emplace(
		"/a/data.json",
		R"json({
	"Version": "3B5589D2-D9AF-40A8-BC40-574DAB6FFC57",
	"Id": 5,
	"Title": "Prof. Dr.",
	"FirstNames": [
		"Philippa",
		"Rosa",
		"Polly"
	],
	"TitleOfNobility": "von und zu",
	"LastNames": [
		"Perry",
		"Figueroa",
		"Russell"
	],
	"Gender": "male",
	"DateOfBirth": "04.05.1996",
	"PlaceOfBirth": "PlaceA",
	"DateOfDeath": "11.11.2021",
	"PlaceOfDeath": "PlaceB",
	"Profession": ["AWork", "BWork"],
	"PlaceOfResidence": [
		{
			"Name": "Place A",
			"StartDate": "04.05.1996",
			"EndDate": "05.05.1996"
		},
		{
			"Name": "Place B",
			"StartDate": "06.05.1996"
		}
	],
	"Audio": "Sunday_plans.mp3",
	"Images":[
		"Trohnsaal.png"
	],
	"Remarks": "RemarkA"
})json");

	AutoFree trace = CreateNoOpTrace();
	AutoFree person = CreateJSONPerson("", platform, trace);

	Person result = person->GetPerson(person, 5, trace); // NOLINT

	CHECK(result.id, 5);
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
	platform.myFolders.emplace("", "abc");
	platform.myFiles.emplace(
		"/abc/data.json",
		R"json({
	"Version": "3B5589D2-D9AF-40A8-BC40-574DAB6FFC57",
	"Id": 5,
	"Audio": "Sunday_plans.mp3",
	"Images":[
		"Trohnsaal.png",
		"Bauhaus.png"
	]
})json");

	AutoFree trace = CreateNoOpTrace();
	int audioLoadedCount = 0;
	platform.myAudioDeprecates["/abc/Sunday_plans.mp3"] = [&]() { audioLoadedCount++; };

	AutoFree person = CreateJSONPerson("", platform, trace);
	person->PlayPerson(person, 5, trace); // NOLINT

	return !platform.myUnexpectedAudio && audioLoadedCount == 1;
}

bool
OpenImageTriggersCorrectPathOnPlatform() {
	MockPlatform platform{};
	platform.myFolders.emplace("", "abc");
	platform.myFiles.emplace(
		"/abc/data.json",
		R"json({
	"Version": "3B5589D2-D9AF-40A8-BC40-574DAB6FFC57",
	"Id": 5,
	"Audio": "Sunday_plans.mp3",
	"Images":[
		"Trohnsaal.png",
		"Bauhaus.png"
	]
})json");

	AutoFree trace = CreateNoOpTrace();
	int firstImageLoadedCount = 0;
	int secondImageLoadedCount = 0;
	platform.myImageDeprecates["/abc/Trohnsaal.png"] = [&]() { firstImageLoadedCount++; };
	platform.myImageDeprecates["/abc/Bauhaus.png"] = [&]() { secondImageLoadedCount++; };

	AutoFree person = CreateJSONPerson("", platform, trace);
	person->ShowImages(person, 5, trace); // NOLINT

	CHECK(platform.myUnexpectedImage, false);
	CHECK(firstImageLoadedCount, 1);
	CHECK(secondImageLoadedCount, 1);
	return true;
}

bool
JSONPersonSuit() {
	bool result = true;
	std::cout << "[ SUITE ] JSONPerson\n";
	result &= RUN(JSONPersonIsZeroIfNoFoldersExist);
	result &= RUN(JSONPersonIsAmountOfFolders);
	result &= RUN(CanRetreavePersonIds);
	result &= RUN(PersonDataCanBeRetreaved);
	result &= RUN(PlayMusicTriggersCorrectPathOnPlatform);
	result &= RUN(OpenImageTriggersCorrectPathOnPlatform);
	return result;
}
