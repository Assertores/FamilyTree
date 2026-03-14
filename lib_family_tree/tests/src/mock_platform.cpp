#include "mock_platform.hpp"

#include <cstdlib>
#include <cstring>
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

MockPlatform::MockPlatform() {
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

MockPlatform::operator IPlatform*() { return &myInterface.myInterface; }

char*
MockPlatform::GetFolders(const char* aPath) {
	auto folders = myFolders.equal_range(aPath);
	if (folders.first == myFolders.end()) {
		myUnexpectedFolder = true;
		return (char*)calloc(2, 1); // NOLINT
	}

	// NOTE: intential leak of memory, as the user is expected to call free on this object.
	std::string result;
	for (auto iterator = folders.first; iterator != folders.second; iterator++) {
		result += iterator->second;
		result += (char)0; // insert intermediate null terminator.
	}
	// result already has two null terminators at the end, so it ready to be returned.

	// NOTE: intential leak of memory, as the user is expected to call free on this object.
	// NOLINTBEGIN
	char* c_str = (char*)malloc(result.size() + 1);
	if (c_str) {
		memcpy(c_str, result.c_str(), result.size() + 1);
	}
	// NOLINTEND
	return c_str;
}

char*
MockPlatform::ReadFile(const char* aPath) {
	auto file = myFiles.find(aPath);
	if (file == myFiles.end()) {
		myUnexpectedFolder = true;
		return (char*)calloc(1, 1); // NOLINT
	}

	// NOTE: intential leak of memory, as the user is expected to call free on this object.
	// NOLINTBEGIN
	char* c_str = (char*)malloc(file->second.size() + 1);
	if (c_str) {
		memcpy(c_str, file->second.c_str(), file->second.size() + 1);
	}
	// NOLINTEND
	return c_str;
}

void // NOLINTNEXTLINE(readability-non-const-parameter,-warnings-as-errors)
MockPlatform::FreeString(char* aString) {
	free(aString); // NOLINT
}

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
