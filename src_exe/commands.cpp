#include "commands.hpp"

#include <family_tree/api.h>

#include <iostream>
#include <vector>

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
	, myPlatform(CreateDefaultPlatform(aContext)) {}

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
	return aCommand == "print-person" || aCommand == "p";
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
	std::cout << "print-person [id], p [id]: prints the detailed informations of a person. id "
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

SearchPeople::SearchPeople(Context* aContext)
	: myContext(aContext) {}

bool
SearchPeople::IsCommand(std::string_view aCommand) {
	return aCommand == "search" || aCommand == "s";
}

void
SearchPeople::ExecuteCommand(const std::string& aLine) {
	size_t minMatches = 1;

	std::string copy = aLine;
	size_t begin = 0;
	for (size_t i = 0; i < copy.size(); i++) {
		if (std::isspace(copy[i])) {
			copy[i] = '\0';
			begin = i + 1;
			break;
		}
	}
	if (begin == 0) {
		return;
	}
	minMatches = std::stoi(copy.c_str());

	Person prototype{};
	std::vector<const char*> firstNames;
	std::vector<const char*> lastNames;
	for (size_t i = begin; i < copy.size(); i++) {
		if (copy.substr(i).rfind("-t=", 0) == 0) {
			copy[i - 1] = '\0';
			prototype.title = copy.c_str() + i + 3;
			continue;
		}
		if (copy.substr(i).rfind("-n=", 0) == 0) {
			copy[i - 1] = '\0';
			firstNames.push_back(copy.c_str() + i + 3);
			continue;
		}
		if (copy.substr(i).rfind("-a=", 0) == 0) {
			copy[i - 1] = '\0';
			prototype.titleOfNobility = copy.c_str() + i + 3;
			continue;
		}
		if (copy.substr(i).rfind("-l=", 0) == 0) {
			copy[i - 1] = '\0';
			lastNames.push_back(copy.c_str() + i + 3);
			continue;
		}
		if (copy.substr(i).rfind("-g=", 0) == 0) {
			copy[i - 1] = '\0';
			prototype.gender = copy.c_str() + i + 3;
			continue;
		}
		if (copy.substr(i).rfind("-bd=", 0) == 0) {
			copy[i - 1] = '\0';
			prototype.dateOfBirth = copy.c_str() + i + 4;
			continue;
		}
		if (copy.substr(i).rfind("-bp=", 0) == 0) {
			copy[i - 1] = '\0';
			prototype.placeOfBirth = copy.c_str() + i + 4;
			continue;
		}
		if (copy.substr(i).rfind("-dd=", 0) == 0) {
			copy[i - 1] = '\0';
			prototype.dateOfDeath = copy.c_str() + i + 4;
			continue;
		}
		if (copy.substr(i).rfind("-dp=", 0) == 0) {
			copy[i - 1] = '\0';
			prototype.placeOfDeath = copy.c_str() + i + 4;
			continue;
		}
		if (copy.substr(i).rfind("-r=", 0) == 0) {
			copy[i - 1] = '\0';
			prototype.remark = copy.c_str() + i + 3;
			continue;
		}
	}
	prototype.firstNameCount = firstNames.size();
	prototype.firstNames = firstNames.data();
	prototype.lastNameCount = lastNames.size();
	prototype.lastNames = lastNames.data();

	size_t count = 0;
	auto array = GetPersonsMatchingPattern(myContext, prototype, minMatches, &count);
	std::vector<Person> matches{array, array + count};
	bool first = true;
	std::cout << "ids: ";
	for (const auto& it : matches) {
		if (!first) {
			std::cout << ", ";
		}
		first = false;
		std::cout << it.id << " (" << it.firstNames[0] << ' ' << it.lastNames[0] << ')';
	}
	std::cout << '\n';
}

void
SearchPeople::PrintHelp() {
	std::cout << "search [matches], s [matches]: searches for people who match at least [matches] "
				 "amount of the parameters. not all parameters need to be given. [matches] needs "
				 "to be an integer.\n"
				 "    -t=[title]: the title of the person.\n"
				 "    -n=[first name]: all people with that first name. this command can be given "
				 "multiple times.\n"
				 "    -a=[title of nobility]: all people who have this title of nobility.\n"
				 "    -l=[last name]: all people with that last name. this command can be given "
				 "multiple times.\n"
				 "    -g=[gender]: all people who have this gender.\n"
				 "    -bd=[birthday]: all people who where born at that day.\n"
				 "    -bp=[birthplace]: all people who where born at that place.\n"
				 "    -dd=[birthday]: all people who died at that day.\n"
				 "    -dp=[birthplace]: all people who died at that place.\n"
				 "    -r=[remarks]: all people who have this remark.\n";
}
