#pragma once

#include <family_tree/types.h>

#include <functional>
#include <map>
#include <string>

class AbstractPlatform;
struct C_Platform {
	IPlatform myInterface;
	AbstractPlatform* myThis;
};

class AbstractPlatform {
public:
	AbstractPlatform();
	operator IPlatform*();
	virtual ~AbstractPlatform() = default;

	virtual char* GetFolders(const char* aPath) { return "\0"; }
	virtual char* ReadFile(const char* aPath) { return ""; }
	virtual void FreeString(char* aString) {}
	virtual void OpenAudio(const char* aPath) {}
	virtual void OpenImage(const char* aPath) {}

private:
	C_Platform myInterface;
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
