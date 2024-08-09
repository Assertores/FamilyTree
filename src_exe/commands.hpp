#pragma once

#include <family_tree/types.h>

#include <set>
#include <string>
#include <string_view>
#include <vector>

class ICommand {
public:
	ICommand();
	virtual ~ICommand();

	ICommand(const ICommand&) = delete;
	ICommand(ICommand&&) = delete;
	ICommand& operator=(const ICommand&) = delete;
	ICommand& operator=(ICommand&&) = delete;

	static bool Execute(std::string_view aCommand, const std::string& aLine);

	virtual bool IsCommand(std::string_view aCommand) = 0;
	virtual void ExecuteCommand(const std::string& aLine) = 0;
	virtual void PrintHelp() = 0;

protected:
	inline static std::set<ICommand*> myCommands{};
	inline static bool myQuitFlag = false;
};

class Help : public ICommand {
public:
	bool IsCommand(std::string_view aCommand) override;
	void ExecuteCommand(const std::string& aLine) override;
	void PrintHelp() override;
};

class Exit : public ICommand {
public:
	bool IsCommand(std::string_view aCommand) override;
	void ExecuteCommand(const std::string& aLine) override;
	void PrintHelp() override;
};

class AddData : public ICommand {
public:
	explicit AddData(Context* aContext);

	bool IsCommand(std::string_view aCommand) override;
	void ExecuteCommand(const std::string& aLine) override;
	void PrintHelp() override;

private:
	Context* myContext;
	IPlatform* myPlatform;
};

class PrintPerson : public ICommand {
public:
	explicit PrintPerson(Context* aContext);

	bool IsCommand(std::string_view aCommand) override;
	void ExecuteCommand(const std::string& aLine) override;
	void PrintHelp() override;

private:
	Context* myContext;

	static void PrivPrintName(size_t aCount, const char* const* aNames);
};

class PlayAudio : public ICommand {
public:
	explicit PlayAudio(Context* aContext);

	bool IsCommand(std::string_view aCommand) override;
	void ExecuteCommand(const std::string& aLine) override;
	void PrintHelp() override;

private:
	Context* myContext;
};

class ShowImages : public ICommand {
public:
	explicit ShowImages(Context* aContext);

	bool IsCommand(std::string_view aCommand) override;
	void ExecuteCommand(const std::string& aLine) override;
	void PrintHelp() override;

private:
	Context* myContext;
};

class SearchPeople : public ICommand {
public:
	explicit SearchPeople(Context* aContext);

	bool IsCommand(std::string_view aCommand) override;
	void ExecuteCommand(const std::string& aLine) override;
	void PrintHelp() override;

private:
	Context* myContext;

	static Person PrivCreatePrototype(
		std::string aLine,
		size_t aBegin,
		std::vector<const char*>& aOutFirstNames,
		std::vector<const char*>& aOutLastNames);
};

class PeopleRelation : public ICommand {
public:
	explicit PeopleRelation(Context* aContext);

	bool IsCommand(std::string_view aCommand) override;
	void ExecuteCommand(const std::string& aLine) override;
	void PrintHelp() override;

private:
	Context* myContext;
};

class PrintTree : public ICommand {
public:
	explicit PrintTree(Context* aContext);

	bool IsCommand(std::string_view aCommand) override;
	void ExecuteCommand(const std::string& aLine) override;
	void PrintHelp() override;

private:
	Context* myContext;
};
