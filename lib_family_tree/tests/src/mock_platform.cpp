#include "mock_platform.hpp"

#include <cstdlib>
#include <string_view>

// NOLINTBEGIN
IPlatform*
AbstractPlatform_Copy(IPlatform* aThis, ITrace* aTrace) {
	return aThis;
}

char*
AbstractPlatform_GetFolders(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	return ((CPlatform*)aThis)->myThis->GetFolders(aPath);
}

char*
AbstractPlatform_ReadFile(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	return ((CPlatform*)aThis)->myThis->ReadFile(aPath);
}

void
AbstractPlatform_FreeString(IPlatform* aThis, char* aString, ITrace* aTrace) {
	((CPlatform*)aThis)->myThis->FreeString(aString);
}

void
AbstractPlatform_OpenAudio(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	((CPlatform*)aThis)->myThis->OpenAudio(aPath);
}

void
AbstractPlatform_OpenImage(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	((CPlatform*)aThis)->myThis->OpenImage(aPath);
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
// NOLINTEND

AbstractPlatform::operator IPlatform*() { return &myInterface.myInterface; }

char*
MockPlatform::GetFolders(const char* aPath) {
	const char backing[] = "abc\0def\0"; // NOLINT std::array is unable to correctly deduce type
	auto* copy = (new std::string(std::begin(backing), std::end(backing)))->data();
	return copy;
}

char*
MockPlatform::ReadFile(const char* aPath) {
	if (aPath == std::string_view("/abc/data.json")) {
		const auto* backing =
			R"json({
	"Version": "3B5589D2-D9AF-40A8-BC40-574DAB6FFC57",
	"Id": 5,
	"Title" : "Prof. Dr.",
	"FirstNames" : [
		"Philippa",
		"Rosa",
		"Polly"
	],
	"TitleOfNobility" : "von und zu",
	"LastNames" : [
		"Perry",
		"Figueroa",
		"Russell"
	],
	"Gender" : "male",
	"DateOfBirth" : "04.05.1996",
	"PlaceOfBirth" : "PlaceA",
	"DateOfDeath" : "11.11.2021",
	"PlaceOfDeath" : "PlaceB",
	"Audio" : "Sunday_plans.mp3",
	"Images" : [
		"Trohnsaal.png",
		"Bauhaus.png"
	] ,
	"Remarks": "RemarkA"
})json";
		return (new std::string(backing))->data();
	}
	if (aPath == std::string_view("/def/data.json")) {
		const auto* backing =
			R"json({"Version": "3B5589D2-D9AF-40A8-BC40-574DAB6FFC57","Id": 32})json";
		return (new std::string(backing))->data();
	}
	const auto* backing = "id1,id2,type\n5,32,StrictlyLower";
	return (new std::string(backing))->data();
}

void
MockPlatform::FreeString(char* aString) {}

void
MockPlatform::OpenAudio(const char* aPath) {
	auto expectation = myAudioDeprecates.find(aPath);
	if (expectation == myAudioDeprecates.end()) {
		myUnexpectedAudio = true;
		return;
	}
	expectation->second();
}

void
MockPlatform::OpenImage(const char* aPath) {
	auto expectation = myImageDeprecates.find(aPath);
	if (expectation == myImageDeprecates.end()) {
		myUnexpectedImage = true;
		return;
	}
	expectation->second();
}

char*
CsvPlatform::ReadFile(const char* aPath) {
	const auto* backing =
		"id1,id2,type\n5,32,StrictlyLower\n13,8,StrictlyLower\n5,13,StrictlyLower\n32,9,"
		"StrictlyLower";
	return (new std::string(backing))->data();
}
