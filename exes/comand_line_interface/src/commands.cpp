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
	std::cout << "Professions: " << person.professions[0];
	for (int i = 1; i < person.professionCount; i++) {
		std::cout << ", " << person.professions[i];
	}
	std::cout << '\n';
	if (person.placeOfResidenceCount > 0) {
		std::cout << "Residence:\n";
		for (int i = 0; i < person.placeOfResidenceCount; i++) {
			std::cout << "    " << person.placeOfResidences[i].name
					  << " from: " << person.placeOfResidences[i].startDate
					  << " to: " << person.placeOfResidences[i].endDate << '\n';
		}
	}
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

	std::vector<const char*> firstNames;
	std::vector<const char*> lastNames;
	Person prototype = PrivCreatePrototype(aLine, begin, firstNames, lastNames);

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
SearchPeople::PrivCreatePrototype(
	std::string aLine,
	size_t aBegin,
	std::vector<const char*>& aOutFirstNames,
	std::vector<const char*>& aOutLastNames) {
	Person prototype{};
	for (size_t i = aBegin; i < aLine.size(); i++) {
		if (aLine.substr(i).rfind("-t=", 0) == 0) {
			aLine[i - 1] = '\0';
			prototype.title = aLine.c_str() + i + 3;
			continue;
		}
		if (aLine.substr(i).rfind("-n=", 0) == 0) {
			aLine[i - 1] = '\0';
			aOutFirstNames.push_back(aLine.c_str() + i + 3);
			continue;
		}
		if (aLine.substr(i).rfind("-a=", 0) == 0) {
			aLine[i - 1] = '\0';
			prototype.titleOfNobility = aLine.c_str() + i + 3;
			continue;
		}
		if (aLine.substr(i).rfind("-l=", 0) == 0) {
			aLine[i - 1] = '\0';
			aOutLastNames.push_back(aLine.c_str() + i + 3);
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
	prototype.firstNameCount = aOutFirstNames.size();
	prototype.firstNames = aOutFirstNames.data();
	prototype.lastNameCount = aOutLastNames.size();
	prototype.lastNames = aOutLastNames.data();
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

class GraphBuilderData;
struct CGraphBuilderStrategy {
	GraphBuilderStrategy myInterface{};
	GraphBuilderData* myThis{};
};

struct GraphBuilderData {
public:
	GraphBuilderData();
	operator GraphBuilderStrategy*(); // NOLINT(hicpp-explicit-conversions) to seamlessly interact
									  // with c API

	void PersonInGenerationStrategy(PersonId aId, int aGeneration);
	void FamilieStrategy(std::vector<PersonId> aParents, std::vector<PersonId> aChilds);

	// NOLINTBEGIN [cppcoreguidelines-non-private-member-variables-in-classes]
	int myFirstGeneration = 0;
	std::list<std::vector<std::shared_ptr<graph::IPerson>>> myGenerations{};
	std::map<PersonId, std::pair<int, std::shared_ptr<graph::IPerson>>> myPersons;
	std::set<graph::FamilieBuilder> myFamilieBuilders{};
	Context* myContext;
	// NOLINTEND

private:
	CGraphBuilderStrategy myInterface;
};

// NOLINTBEGIN
void
GraphBuilderData_PersonInGenerationStrategy(
	GraphBuilderStrategy* aThis,
	PersonId aId,
	int aGeneration) {
	((CGraphBuilderStrategy*)aThis)->myThis->PersonInGenerationStrategy(aId, aGeneration);
}

void
GraphBuilderData_FamilieStrategy(
	GraphBuilderStrategy* aThis,
	PersonId* aParents,
	size_t aParentCount,
	PersonId* aChilds,
	size_t aChildCount) {
	((CGraphBuilderStrategy*)aThis)
		->myThis->FamilieStrategy(
			{aParents, aParents + aParentCount},
			{aChilds, aChilds + aChildCount});
}

GraphBuilderData::GraphBuilderData() {
	myInterface.myInterface.PersonInGenerationStrategy =
		GraphBuilderData_PersonInGenerationStrategy;
	myInterface.myInterface.FamilieStrategy = GraphBuilderData_FamilieStrategy;
	myInterface.myThis = this;
}
// NOLINTEND

GraphBuilderData::operator GraphBuilderStrategy*() { return &myInterface.myInterface; }

void
GraphBuilderData::PersonInGenerationStrategy(PersonId aId, int aGeneration) {
	while (aGeneration < myFirstGeneration) {
		myGenerations.emplace_front();
		myFirstGeneration--;
	}
	while (aGeneration - myFirstGeneration >= myGenerations.size()) {
		myGenerations.emplace_back();
	}
	auto iterator = myGenerations.begin();
	for (size_t i = 0; i < (aGeneration - myFirstGeneration); i++) {
		iterator++;
	}
	auto person = graph::IPerson::CreatePerson(
		GetPerson(myContext, aId, nullptr),
		[&](const char* aString) { return IsDefaultString(myContext, aString) != 0; });
	iterator->push_back(person);
	myPersons[aId] = std::make_pair(aGeneration, person);
}

void
GraphBuilderData::FamilieStrategy(std::vector<PersonId> aParents, std::vector<PersonId> aChilds) {
	graph::FamilieBuilder familie;
	for (const auto& parentId : aParents) {
		auto parent = myPersons.find(parentId);
		if (parent == myPersons.end()) {
			continue;
		}
		familie.AddParent(parent->second.second.get(), parent->second.first);
	}
	for (const auto& childId : aChilds) {
		auto child = myPersons.find(childId);
		if (child == myPersons.end()) {
			continue;
		}
		familie.AddChild(child->second.second.get(), child->second.first);
	}

	myFamilieBuilders.insert(familie);
}

void
PrintTree::ExecuteCommand(const std::string& aLine) {
	PersonId requestedIds = 0;
	size_t distance = 0;
	std::stringstream(aLine) >> requestedIds >> distance;

	GraphBuilderData data{};
	data.myContext = myContext;

	auto trace = AbstractTrace::CreatePrintingTrace();
	BuildGraph(myContext, requestedIds, distance, data, *trace);

	std::vector<std::shared_ptr<graph::IFamilie>> families;
	for (size_t i = 0; i < data.myGenerations.size(); i++) {
		auto familie = graph::IFamilie::CreateProxy();
		families.push_back(familie);

		auto iterator = data.myGenerations.begin();
		for (size_t j = 0; j < i; j++) {
			iterator++;
		}

		for (const auto& builder : data.myFamilieBuilders) {
			auto proxy =
				builder.BuildGeneration(data.myFirstGeneration + static_cast<int>(i), *familie);
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

	size_t i = 0; // NOLINT
	for (const auto& generation : data.myGenerations) {
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

void
PrintTree::PrintHelp() {
	std::cout << "print-tree [id] [dist], t [id] [dist]: prints a familytree around person [id] "
				 "with all persons at max [dist] steps removed from that person.\n";
}
