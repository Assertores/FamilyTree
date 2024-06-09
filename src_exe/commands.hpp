#pragma once

#include <family_tree/types.h>

#include <set>
#include <string>
#include <string_view>

class ICommand {
public:
	ICommand();
	virtual ~ICommand();

	static bool Execute(std::string_view aCommand, const std::string& aLine);

	virtual bool IsCommand(std::string_view aCommand) = 0;
	virtual void ExecuteCommand(const std::string& aLine) = 0;
	virtual void PrintHelp() = 0;

protected:
	static std::set<ICommand*> myCommands;
	static bool myQuitFlag;
};

class Help : public ICommand {
public:
	bool IsCommand(std::string_view aCommand);
	void ExecuteCommand(const std::string& aLine);
	void PrintHelp();
};

class Exit : public ICommand {
public:
	bool IsCommand(std::string_view aCommand);
	void ExecuteCommand(const std::string& aLine);
	void PrintHelp();
};

class AddData : public ICommand {
public:
	AddData(Context* aContext);

	bool IsCommand(std::string_view aCommand);
	void ExecuteCommand(const std::string& aLine);
	void PrintHelp();

private:
	Context* myContext;
	IPlatform* myPlatform;
};

class PrintPerson : public ICommand {
public:
	PrintPerson(Context* aContext);

	bool IsCommand(std::string_view aCommand);
	void ExecuteCommand(const std::string& aLine);
	void PrintHelp();

private:
	Context* myContext;

	void PrivPrintName(size_t aCount, const char* const* aNames);
};
