#include "commands.hpp"

#include <family_tree/api.h>

#ifndef NDEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <iostream>

int
main() { // NOLINT(bugprone-exception-escape)
	auto* context = Create(nullptr);

	Help help;
	Exit exit;
	AddData addData(context);
	PrintPerson printPerson(context);
	ShowImages showImages(context);
	PlayAudio playAudio(context);
	SearchPeople searchPeople(context);
	PeopleRelation peopleRelation(context);
	PrintTree printTree(context);

	bool quit = false;
	while (!quit) {
		std::cout << ">> ";
		std::string command{};
		std::cin >> command;
		std::string line{};
		std::getline(std::cin, line);

		size_t nonWhitespaceCharacter = 0;
		for (size_t i = 0; i < line.size(); i++) {
			if (std::isspace(line[i]) == 0) {
				nonWhitespaceCharacter = i;
				break;
			}
		}
		line = line.substr(nonWhitespaceCharacter);

		quit = ICommand::Execute(command, line);
	}

	Free(context, nullptr);
	return 0;
}
