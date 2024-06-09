#include "commands.hpp"

#include <family_tree/api.h>

#ifndef NDEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <iostream>

int
main() {
	auto context = Create();

	Help help;
	Exit exit;
	AddData addData(context);
	PrintPerson printPerson(context);
	ShowImages showImages(context);
	PlayAudio playAudio(context);
	SearchPeople searchPeople(context);

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
