#include "family_tree/api.h"

#ifndef NDEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <iostream>
#include <set>
#include <string>
#include <string_view>

class ICommand {
public:
	ICommand() { myCommands.insert(this); }
	virtual ~ICommand() { myCommands.erase(this); }

	static bool Execute(std::string_view aCommand, const std::string& aLine) {
		for (const auto& it : myCommands) {
			if (it->IsCommand(aCommand)) {
				it->ExecuteCommand(aLine);
				return myQuitFlag;
			}
		}
		std::cout << "Command not found: " << aCommand << '\n';
		for (const auto& it : myCommands) {
			it->PrintHelp();
		}
		return true;
	}

	virtual bool IsCommand(std::string_view aCommand) = 0;
	virtual void ExecuteCommand(const std::string& aLine) = 0;
	virtual void PrintHelp() = 0;

protected:
	static std::set<ICommand*> myCommands;
	static bool myQuitFlag;
};

std::set<ICommand*> ICommand::myCommands = {};
bool ICommand::myQuitFlag = false;

class Help : public ICommand {
public:
	bool IsCommand(std::string_view aCommand) { return aCommand == "help" || aCommand == "h"; }
	void ExecuteCommand(const std::string& aLine) {
		for (const auto& it : myCommands) {
			it->PrintHelp();
		}
	}
	void PrintHelp() { std::cout << "help, h: shows this help\n"; }
};

class Exit : public ICommand {
public:
	bool IsCommand(std::string_view aCommand) {
		return aCommand == "exit" || aCommand == "quit" || aCommand == "e" || aCommand == "q";
	}
	void ExecuteCommand(const std::string& aLine) { myQuitFlag = true; }
	void PrintHelp() { std::cout << "exit, quit, e, q: exit the program\n"; }
};

class AddData : public ICommand {
public:
	AddData(Context* aContext)
		: myContext(aContext)
		, myPlatform(CreateDefaultIo(aContext)) {}

	bool IsCommand(std::string_view aCommand) { return aCommand == "add"; }
	void ExecuteCommand(const std::string& aLine) {
		auto relation = CreateCSVRelations(myContext, aLine.c_str(), myPlatform);
		auto person = CreateJSONPersonals(myContext, aLine.c_str(), myPlatform);
		auto dataProvider = CreateDataProvider(myContext, relation, person);
		AddDataProvider(myContext, dataProvider);
	}
	void PrintHelp() { std::cout << "add: adds a folder with data to this programms instance\n"; }

private:
	Context* myContext;
	IPlatform* myPlatform;
};

class PrintPerson : public ICommand {
public:
	PrintPerson(Context* aContext)
		: myContext(aContext) {}

	bool IsCommand(std::string_view aCommand) {
		return aCommand == "print-person" || aCommand == "d";
	}
	void ExecuteCommand(const std::string& aLine) {
		auto person = GetPerson(myContext, std::stoi(aLine));

		std::cout << "ID: " << person.id << '\n';
		std::cout << "Name: ";
		if (person.firstNameCount == 0 && person.lastNameCount == 0) {
			std::cout << "Unnamed\n";
		} else {
			if (!IsDefaultString(myContext, person.title)) {
				std::cout << person.title << ' ';
			}
			PrivPrintName(person.firstNameCount, person.firstNames);
			std::cout << ", ";
			if (!IsDefaultString(myContext, person.titleOfNobility)) {
				std::cout << person.titleOfNobility << ' ';
			}
			PrivPrintName(person.lastNameCount, person.lastNames);
			std::cout << '\n';
		}
		std::cout << "Gender: " << person.gender << '\n';
		std::cout << "Date of birth: " << person.dateOfBirth << " at: " << person.placeOfBirth
				  << '\n';
		std::cout << "Date of death: " << person.dateOfDeath << " at: " << person.placeOfDeath
				  << '\n';
		std::cout << "Remarks:\n" << person.remark << '\n';
	}
	void PrintHelp() {
		std::cout << "print-person [id], d [id]: prints the detailed informations of a person. id "
					 "needs to be an interger\n";
	}

private:
	Context* myContext;

	void PrivPrintName(size_t aCount, const char* const* aNames) {
		if (aCount <= 0) {
			return;
		}
		std::cout << aNames[0];
		if (aCount > 1) {
			std::cout << " (";
		}
		for (size_t i = 1; i < aCount; i++) {
			if (i != 1) {
				std::cout << ", ";
			}
			std::cout << aNames[i];
		}
		if (aCount > 1) {
			std::cout << ')';
		}
	}
};

int
main() {
	auto context = Create();

	Help help;
	Exit exit;
	AddData addData(context);
	PrintPerson printPerson(context);

	bool quit = false;
	while (!quit) {
		std::cout << ">> ";
		std::string command{};
		std::cin >> command;
		std::string line{};
		std::getline(std::cin, line);

		size_t nonWhitespaceCharacter = 0;
		for (size_t i = 0; i < line.size(); i++) {
			if (!std::isspace(line[i])) {
				nonWhitespaceCharacter = i;
				break;
			}
		}
		line = line.substr(nonWhitespaceCharacter);

		quit = ICommand::Execute(command, line);
	}

	Free(context);
	return 0;
}
