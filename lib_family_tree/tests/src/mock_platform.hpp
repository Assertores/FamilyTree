#pragma once

#include <family_tree/types.h>

#include <array>
#include <functional>
#include <map>
#include <string>

class MockPlatform;
struct CPlatform {
	IPlatform myInterface{};
	MockPlatform* myThis{};
};

class MockPlatform {
public:
	MockPlatform();
	operator IPlatform*(); // NOLINT(hicpp-explicit-conversions) to seamlessly interact with c API

	char* GetFolders(const char* aPath);
	char* ReadFile(const char* aPath);
	void FreeString(char* aString);
	void OpenAudio(const char* aPath);
	void OpenImage(const char* aPath);

	// NOLINTBEGIN
	// to ease testing direct access is fine.
	std::multimap<std::string, std::string> myFolders;
	std::map<std::string, std::string> myFiles;
	std::map<std::string, std::function<void()>> myAudioDeprecates;
	std::map<std::string, std::function<void()>> myImageDeprecates;

	bool myUnexpectedAudio = false;
	bool myUnexpectedImage = false;
	bool myUnexpectedFolder = false;
	bool myUnexpectedFile = false;
	// NOLINTEND

private:
	CPlatform myInterface;
};
