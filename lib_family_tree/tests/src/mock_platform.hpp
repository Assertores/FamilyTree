#pragma once

#include <family_tree/types.h>

#include <array>
#include <functional>
#include <map>
#include <string>

class AbstractPlatform;
struct CPlatform {
	IPlatform myInterface{};
	AbstractPlatform* myThis{};
};

class AbstractPlatform {
public:
	AbstractPlatform();
	operator IPlatform*(); // NOLINT(hicpp-explicit-conversions) to seamlessly interact with c API
	virtual ~AbstractPlatform() = default;

	// TODO: fix default implimentation
	virtual char* GetFolders(const char* aPath) { return "\0"; } // NOLINT
	virtual char* ReadFile(const char* aPath) { return ""; }	 // NOLINT
	virtual void FreeString(char* aString) {}
	virtual void OpenAudio(const char* aPath) {}
	virtual void OpenImage(const char* aPath) {}

private:
	CPlatform myInterface;
};

class MockPlatform : public AbstractPlatform {
public:
	char* GetFolders(const char* aPath) override;
	char* ReadFile(const char* aPath) override;
	void FreeString(char* aString) override;
	void OpenAudio(const char* aPath) override;
	void OpenImage(const char* aPath) override;

	std::map<std::string, std::function<void()>> myAudioDeprecates;
	std::map<std::string, std::function<void()>> myImageDeprecates;

	bool myUnexpectedAudio = false;
	bool myUnexpectedImage = false;
};

class CsvPlatform : public AbstractPlatform {
public:
	char* ReadFile(const char* aPath) override;
};
