#include "commands.hpp"

#include <family_tree/api.h>

#include <iostream>

std::set<ICommand*> ICommand::myCommands = {};
bool ICommand::myQuitFlag = false;

ICommand::ICommand() { myCommands.insert(this); }
ICommand::~ICommand() { myCommands.erase(this); }

bool
ICommand::Execute(std::string_view aCommand, const std::string& aLine) {
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

bool
Help::IsCommand(std::string_view aCommand) {
	return aCommand == "help" || aCommand == "h";
}

void
Help::ExecuteCommand(const std::string& aLine) {
	for (const auto& it : myCommands) {
		it->PrintHelp();
	}
}

void
Help::PrintHelp() {
	std::cout << "help, h: shows this help\n";
}

bool
Exit::IsCommand(std::string_view aCommand) {
	return aCommand == "exit" || aCommand == "quit" || aCommand == "e" || aCommand == "q";
}

void
Exit::ExecuteCommand(const std::string& aLine) {
	myQuitFlag = true;
}

void
Exit::PrintHelp() {
	std::cout << "exit, quit, e, q: exit the program\n";
}

AddData::AddData(Context* aContext)
	: myContext(aContext)
	, myPlatform(CreateDefaultIo(aContext)) {}

bool
AddData::IsCommand(std::string_view aCommand) {
	return aCommand == "add";
}

void
AddData::ExecuteCommand(const std::string& aLine) {
	auto relation = CreateCSVRelations(myContext, aLine.c_str(), myPlatform);
	auto person = CreateJSONPersonals(myContext, aLine.c_str(), myPlatform);
	auto dataProvider = CreateDataProvider(myContext, relation, person);
	AddDataProvider(myContext, dataProvider);
}
void
AddData::PrintHelp() {
	std::cout << "add: adds a folder with data to this programms instance\n";
}

PrintPerson::PrintPerson(Context* aContext)
	: myContext(aContext) {}

bool
PrintPerson::IsCommand(std::string_view aCommand) {
	return aCommand == "print-person" || aCommand == "d";
}

void
PrintPerson::ExecuteCommand(const std::string& aLine) {
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
	std::cout << "Date of birth: " << person.dateOfBirth << " at: " << person.placeOfBirth << '\n';
	std::cout << "Date of death: " << person.dateOfDeath << " at: " << person.placeOfDeath << '\n';
	std::cout << "Remarks:\n" << person.remark << '\n';
}

void
PrintPerson::PrintHelp() {
	std::cout << "print-person [id], d [id]: prints the detailed informations of a person. id "
				 "needs to be an interger\n";
}

void
PrintPerson::PrivPrintName(size_t aCount, const char* const* aNames) {
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

PlayAudio::PlayAudio(Context* aContext)
	: myContext(aContext) {}

bool
PlayAudio::IsCommand(std::string_view aCommand) {
	return aCommand == "play-audio" || aCommand == "a";
}

void
PlayAudio::ExecuteCommand(const std::string& aLine) {
	PlayPerson(myContext, std::stoi(aLine));
}

void
PlayAudio::PrintHelp() {
	std::cout << "play-audio [id], a [id]: plays the available audio of a person. id needs "
				 "to be an integer\n";
}

ShowImages::ShowImages(Context* aContext)
	: myContext(aContext) {}

bool
ShowImages::IsCommand(std::string_view aCommand) {
	return aCommand == "show-images" || aCommand == "i";
}

void
ShowImages::ExecuteCommand(const std::string& aLine) {
	ShowImagesOfPerson(myContext, std::stoi(aLine));
}

void
ShowImages::PrintHelp() {
	std::cout << "show-images [id], i [id]: opens all available images of a person. id needs to be "
				 "an integer\n";
}
