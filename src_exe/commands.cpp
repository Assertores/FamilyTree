#include "commands.hpp"

#include "graph.hpp"
#include "trace.hpp"

#include <family_tree/api.h>

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <vector>

ICommand::ICommand() { myCommands.insert(this); }
ICommand::~ICommand() { myCommands.erase(this); }

bool
ICommand::Execute(std::string_view aCommand, const std::string& aLine) {
	for (const auto& command : myCommands) {
		if (command->IsCommand(aCommand)) {
			command->ExecuteCommand(aLine);
			return myQuitFlag;
		}
	}
	std::cout << "Command not found: " << aCommand << '\n';
	for (const auto& command : myCommands) {
		command->PrintHelp();
	}
	return false;
}

bool
Help::IsCommand(std::string_view aCommand) {
	return aCommand == "help" || aCommand == "h";
}

void
Help::ExecuteCommand(const std::string& aLine) {
	for (const auto& command : myCommands) {
		command->PrintHelp();
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
	, myPlatform(CreateDefaultPlatform(aContext, nullptr)) {}

bool
AddData::IsCommand(std::string_view aCommand) {
	return aCommand == "add" || aCommand == "data" || aCommand == "d";
}

void
AddData::ExecuteCommand(const std::string& aLine) {
	auto trace = AbstractTrace::CreatePrintingTrace();
	auto* relation = CreateCSVRelations(myContext, aLine.c_str(), myPlatform, *trace);
	auto* person = CreateJSONPersonals(myContext, aLine.c_str(), myPlatform, *trace);
	auto* dataProvider = CreateDataProvider(myContext, relation, person, *trace);
	AddDataProvider(myContext, dataProvider, *trace);
}
void
AddData::PrintHelp() {
	std::cout << "add, data, d: adds a folder with data to this programms instance\n";
}

PrintPerson::PrintPerson(Context* aContext)
	: myContext(aContext) {}

bool
PrintPerson::IsCommand(std::string_view aCommand) {
	return aCommand == "print-person" || aCommand == "p";
}

void
PrintPerson::ExecuteCommand(const std::string& aLine) {
	auto person = GetPerson(myContext, std::stoi(aLine), nullptr);

	std::cout << "ID: " << person.id << '\n';
	std::cout << "Name: ";
	if (person.firstNameCount == 0 && person.lastNameCount == 0) {
		std::cout << "Unnamed\n";
	} else {
		if (IsDefaultString(myContext, person.title) == 0) {
			std::cout << person.title << ' ';
		}
		PrivPrintName(person.firstNameCount, person.firstNames);
		std::cout << ", ";
		if (IsDefaultString(myContext, person.titleOfNobility) == 0) {
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
	PlayPerson(myContext, std::stoi(aLine), nullptr);
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
	ShowImagesOfPerson(myContext, std::stoi(aLine), nullptr);
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
		if (std::isspace(copy[i]) != 0) {
			copy[i] = '\0';
			begin = i + 1;
			break;
		}
	}
	if (begin == 0) {
		return;
	}
	minMatches = std::stoi(copy);

	Person prototype = PrivCreatePrototype(aLine, begin);

	size_t count = 0;
	auto* array = GetPersonsMatchingPattern(myContext, prototype, minMatches, &count, nullptr);
	std::vector<Person> matches{array, array + count};
	bool first = true;
	std::cout << "ids: ";
	for (const auto& match : matches) {
		if (!first) {
			std::cout << ", ";
		}
		first = false;
		std::cout << match.id << " (" << match.firstNames[0] << ' ' << match.lastNames[0] << ')';
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

Person
SearchPeople::PrivCreatePrototype(std::string aLine, size_t aBegin) {
	Person prototype{};
	std::vector<const char*> firstNames;
	std::vector<const char*> lastNames;
	for (size_t i = aBegin; i < aLine.size(); i++) {
		if (aLine.substr(i).rfind("-t=", 0) == 0) {
			aLine[i - 1] = '\0';
			prototype.title = aLine.c_str() + i + 3;
			continue;
		}
		if (aLine.substr(i).rfind("-n=", 0) == 0) {
			aLine[i - 1] = '\0';
			firstNames.push_back(aLine.c_str() + i + 3);
			continue;
		}
		if (aLine.substr(i).rfind("-a=", 0) == 0) {
			aLine[i - 1] = '\0';
			prototype.titleOfNobility = aLine.c_str() + i + 3;
			continue;
		}
		if (aLine.substr(i).rfind("-l=", 0) == 0) {
			aLine[i - 1] = '\0';
			lastNames.push_back(aLine.c_str() + i + 3);
			continue;
		}
		if (aLine.substr(i).rfind("-g=", 0) == 0) {
			aLine[i - 1] = '\0';
			prototype.gender = aLine.c_str() + i + 3;
			continue;
		}
		if (aLine.substr(i).rfind("-bd=", 0) == 0) {
			aLine[i - 1] = '\0';
			prototype.dateOfBirth = aLine.c_str() + i + 4;
			continue;
		}
		if (aLine.substr(i).rfind("-bp=", 0) == 0) {
			aLine[i - 1] = '\0';
			prototype.placeOfBirth = aLine.c_str() + i + 4;
			continue;
		}
		if (aLine.substr(i).rfind("-dd=", 0) == 0) {
			aLine[i - 1] = '\0';
			prototype.dateOfDeath = aLine.c_str() + i + 4;
			continue;
		}
		if (aLine.substr(i).rfind("-dp=", 0) == 0) {
			aLine[i - 1] = '\0';
			prototype.placeOfDeath = aLine.c_str() + i + 4;
			continue;
		}
		if (aLine.substr(i).rfind("-r=", 0) == 0) {
			aLine[i - 1] = '\0';
			prototype.remark = aLine.c_str() + i + 3;
			continue;
		}
	}
	prototype.firstNameCount = firstNames.size();
	prototype.firstNames = firstNames.data();
	prototype.lastNameCount = lastNames.size();
	prototype.lastNames = lastNames.data();
	return prototype;
}

PeopleRelation::PeopleRelation(Context* aContext)
	: myContext(aContext) {}

bool
PeopleRelation::IsCommand(std::string_view aCommand) {
	return aCommand == "show-relations" || aCommand == "r";
}

void
PeopleRelation::ExecuteCommand(const std::string& aLine) {
	size_t requestedIds = std::stoi(aLine);

	size_t count = 0;
	auto* array = GetPersonRelations(myContext, requestedIds, &count, nullptr);
	std::vector<Relation> relations{array, array + count};
	for (const auto& relation : relations) {
		std::cout << relation.id1 << " --";
		if (IsDefaultString(myContext, relation.relationship) == 0) {
			std::cout << relation.relationship;
		}
		std::cout << "-> " << relation.id2 << ": (" << relation.startDate << ", "
				  << relation.endDate << ")\n";
	}
}

void
PeopleRelation::PrintHelp() {
	std::cout << "show-relations [id], r [id]: prints out all relations from a sertain person\n";
}

PrintTree::PrintTree(Context* aContext)
	: myContext(aContext) {}

bool
PrintTree::IsCommand(std::string_view aCommand) {
	return aCommand == "print-tree" || aCommand == "t";
}

// NOLINTBEGIN
void
PrintTree::ExecuteCommand(const std::string& aLine) {
	PersonId requestedIds = 0;
	size_t distance = 0;
	std::stringstream(aLine) >> requestedIds >> distance;

	std::set<PersonId> peopleToShow{};
	peopleToShow.insert(requestedIds);

	for (size_t i = 0; i < distance; i++) {
		std::set<PersonId> newPeopleToShow{};
		for (const auto& personId : peopleToShow) {
			size_t count = 0;
			auto* array = GetPersonRelations(myContext, personId, &count, nullptr);
			for (size_t j = 0; j < count; j++) {
				newPeopleToShow.insert(array[j].id1);
				newPeopleToShow.insert(array[j].id2);
			}
		}
		peopleToShow.insert(newPeopleToShow.begin(), newPeopleToShow.end());
	}

	int firstGeneration = 0;
	std::list<std::vector<std::shared_ptr<graph::IPerson>>> generations;
	std::map<PersonId, std::pair<int, std::shared_ptr<graph::IPerson>>> persons;
	std::set<graph::FamilieBuilder> familieBuilders;

	for (const auto& personId : peopleToShow) {
		auto othersGeneration = GetRelativeGeneration(myContext, requestedIds, personId, nullptr);
		while (othersGeneration < firstGeneration) {
			generations.emplace_front();
			firstGeneration--;
		}
		while (othersGeneration - firstGeneration >= generations.size()) {
			generations.emplace_back();
		}
		auto iterator = generations.begin();
		for (size_t i = 0; i < (othersGeneration - firstGeneration); i++) {
			iterator++;
		}
		auto person = graph::IPerson::CreatePerson(GetPerson(myContext, personId, nullptr));
		iterator->push_back(person);
		persons[personId] = std::make_pair(othersGeneration, person);
	}

	for (const auto& [personId, graphPerson] : persons) {
		size_t count = 0;
		auto* partners = GetPartners(myContext, personId, &count, nullptr);
		for (size_t i = 0; i < count; i++) {
			PersonId parents[] = {personId, partners[i]};
			size_t childCount = 0;
			auto* children = GetCommonChildren(myContext, 2, parents, &childCount, nullptr);

			size_t parentCount = 0;
			auto fullParents =
				GetCommonParents(myContext, childCount, children, &parentCount, nullptr);

			graph::FamilieBuilder familie;
			for (size_t index = 0; index < childCount; index++) {
				auto child = persons.find(children[index]);
				if (child == persons.end()) {
					continue;
				}
				familie.AddChild(child->second.second.get(), child->second.first);
			}
			for (size_t index = 0; index < parentCount; index++) {
				auto parent = persons.find(fullParents[index]);
				if (parent == persons.end()) {
					continue;
				}
				familie.AddParent(parent->second.second.get(), parent->second.first);
			}

			familieBuilders.insert(familie);
		}
	}

	std::vector<std::shared_ptr<graph::IFamilie>> families;
	for (size_t i = 0; i < generations.size(); i++) {
		auto familie = graph::IFamilie::CreateProxy();
		families.push_back(familie);

		auto iterator = generations.begin();
		for (size_t j = 0; j < i; j++) {
			iterator++;
		}

		for (const auto& builder : familieBuilders) {
			auto proxy = builder.BuildGeneration(firstGeneration + i, *familie.get());
			if (proxy == nullptr) {
				continue;
			}
			iterator->emplace_back(proxy);
		}
		if (iterator->empty()) {
			continue;
		}

		for (size_t j = 1; j < iterator->size(); j++) {
			(*iterator)[j]->SetPreviousPort((*iterator)[j - 1].get());
			(*iterator)[j - 1]->SetNextElement((*iterator)[j].get());
		}
	}

	size_t i = 0;
	for (const auto& generation : generations) {
		std::vector<char> canvas{};
		bool notDone = false;
		do { // NOLINT
			notDone = generation[0]->PrintNextLine(canvas);
			std::cout << std::string{canvas.begin(), canvas.end()} << '\n';
			canvas.clear();
		} while (notDone);
		do { // NOLINT
			notDone = families[i]->PrintNextLine(canvas);
			std::cout << std::string{canvas.begin(), canvas.end()} << '\n';
			canvas.clear();
		} while (notDone);
		i++;
	}
}
// NOLINTEND

void
PrintTree::PrintHelp() {
	std::cout << "print-tree [id] [dist], t [id] [dist]: prints a familytree around person [id] "
				 "with all persons at max [dist] steps removed from that person.\n";
}
