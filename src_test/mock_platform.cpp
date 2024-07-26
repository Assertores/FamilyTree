#include "mock_platform.hpp"

#include <stdlib.h>

#include <string_view>

IPlatform*
AbstractPlatform_Copy(IPlatform* aThis, ITrace* aTrace) {
	return aThis;
}

char*
AbstractPlatform_GetFolders(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	return ((C_Platform*)aThis)->myThis->GetFolders(aPath);
}

char*
AbstractPlatform_ReadFile(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	return ((C_Platform*)aThis)->myThis->ReadFile(aPath);
}

void
AbstractPlatform_FreeString(IPlatform* aThis, char* aString, ITrace* aTrace) {
	((C_Platform*)aThis)->myThis->FreeString(aString);
}

void
AbstractPlatform_OpenAudio(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	((C_Platform*)aThis)->myThis->OpenAudio(aPath);
}

void
AbstractPlatform_OpenImage(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	((C_Platform*)aThis)->myThis->OpenImage(aPath);
}

void
AbstractPlatform_Free(IPlatform* aThis, ITrace* aTrace) {
	// TODO: figure this out
}

AbstractPlatform::AbstractPlatform() {
	myInterface.myInterface.Copy = AbstractPlatform_Copy;
	myInterface.myInterface.GetFolders = AbstractPlatform_GetFolders;
	myInterface.myInterface.ReadFile = AbstractPlatform_ReadFile;
	myInterface.myInterface.FreeString = AbstractPlatform_FreeString;
	myInterface.myInterface.OpenAudio = AbstractPlatform_OpenAudio;
	myInterface.myInterface.OpenImage = AbstractPlatform_OpenImage;
	myInterface.myInterface.Free = AbstractPlatform_Free;
	myInterface.myThis = this;
}

AbstractPlatform::operator IPlatform*() { return &myInterface.myInterface; }

char*
MockPlatform::GetFolders(const char* aPath) {
	const char backing[] = "abc\0def\0";
	auto a = (new std::string(std::begin(backing), std::end(backing)))->data();
	return a;
}

char*
MockPlatform::ReadFile(const char* aPath) {
	if (aPath == std::string_view("/abc/data.json")) {
		auto backing =
			R"json({
	"person": 5,
	"title" : "Prof. Dr.",
	"firstNames" : [
		"Philippa",
		"Rosa",
		"Polly"
	],
	"titleOfNobility" : "von und zu",
	"lastNames" : [
		"Perry",
		"Figueroa",
		"Russell"
	],
	"gender" : "male",
	"dateOfBirth" : "04.05.1996",
	"placeOfBirth" : "PlaceA",
	"death" : "11.11.2021",
	"placeOfDeath" : "PlaceB",
	"Audio" : "Sunday_plans.mp3",
	"Images" : [
		"Trohnsaal.png"
		"Bauhaus.png"
	] ,
	"Remarks": "RemarkA"
})json";
		return (new std::string(backing))->data();
	}
	if (aPath == std::string_view("/def/data.json")) {
		auto backing = R"json({\"person\": 32})json";
		return (new std::string(backing))->data();
	}
	auto backing = "id1,id2,type\n5,32,StrictlyLower";
	return (new std::string(backing))->data();
}

void
MockPlatform::FreeString(char* aString) {}

void
MockPlatform::OpenAudio(const char* aPath) {
	auto e = myAudioDeprecates.find(aPath);
	if (e == myAudioDeprecates.end()) {
		myUnexpectedAudio = true;
		return;
	}
	e->second();
}

void
MockPlatform::OpenImage(const char* aPath) {
	auto e = myImageDeprecates.find(aPath);
	if (e == myImageDeprecates.end()) {
		myUnexpectedImage = true;
		return;
	}
	e->second();
}

char*
CsvPlatform::ReadFile(const char* aPath) {
	auto backing = "id1,id2,type\n5,32,StrictlyLower\n13,8,StrictlyLower\n5,13,StrictlyLower\n32,9,StrictlyLower";
	return (new std::string(backing))->data();
}
