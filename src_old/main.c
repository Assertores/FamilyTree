#include "application.h"
#include "application_types.h"
#include "base_types.h"
#include "generic_types.h"
#include "graph.h"
#include "platform.h"
#include "print.h"

#include <stdio.h>

Bool
AddPerson(Graph* aGraph, Person aPerson) {
	Bool result = AppendPerson(&aGraph->persons, aPerson);
	if (result == Duplicate) {
		for (size_t i = 0; i < aGraph->persons.length; i++) {
			if (EqualPerson(&aGraph->persons.data[i], &aPerson) == True) {
				MergePersonalData(&aGraph->persons.data[i], aPerson, True);
				return True;
			}
		}
	}
	FreePerson(aPerson);
	return True;
}

EPerson
FindPerson(const Persons* aData, int aPerson) {
	for (size_t i = 0; i < aData->length; i++) {
		if (aData->data[i].person == aPerson) {
			EPerson result = {0};
			result.data = aData->data[i];
			result.hasData = True;
			return result;
		}
	}
	EPerson result = {0};
	result.hasData = NotFound;
	return result;
}

Bool
InterpreteCommand(FILE* input, Graph* aGraph, Path aCurrentWorkingDirectory) {
	char response[32];
	fscanf(input, "%s", response); // NOLINT
	printf("running command -> %s\n", response);

	if (strcmp(response, "exit") == 0 || strcmp(response, "quit") == 0 || strcmp(response, "e") == 0
		|| strcmp(response, "q") == 0) {
		return True;
	}
	if (strcmp(response, "help") == 0 || strcmp(response, "h") == 0) {
		printf(
			"exit, quit, e, q:               exit the program\n"
			"help, h:                        shows aThis help\n"
			"print-graph, g:                 prints the family tree\n"
			"add-person [path], p [path]:    adds more personal data to aThis program instance. "
			"needs to be a path to a json file\n"
			"add-relations [path], r [path]: adds more relations to aThis program instance. needs "
			"to be a path to a csv file\n"
			"print-person [id], d [id]:      prints the detailed informations to a person. id "
			"needs to be an integer\n"
			"play-audio [id], a [id]:        plays the available audio of a person. id needs to be "
			"an integer\n"
			"show-images [id], i [id]:       opens all available images of a person. id needs to "
			"be an integer\n"
			"macro [path], m [path]:         reads in file and executes commands line by line\n");
	} else if (strcmp(response, "print-graph") == 0 || strcmp(response, "g") == 0) {
		Bool success = PrintGraph(*aGraph);
		if (success != True) {
			printf("%s\n", ErrorToString(success).c_str);
			return False;
		}
	} else if (strcmp(response, "add-person") == 0 || strcmp(response, "p") == 0) {
		char pathbuf[1028];
		fscanf(input, "%s", pathbuf);
		Path path = CreatePath(FromCString(pathbuf));
		if (path.isAbsolut != True) {
			path = GetAbsolut(path, CopyPath(aCurrentWorkingDirectory));
		}

		EPerson newPerson = ReadInPersonalData(path);
		if (newPerson.hasData != True) {
			printf("%s\n", ErrorToString(newPerson.hasData).c_str);
			return False;
		}
		AddPerson(aGraph, newPerson.data);
	} else if (strcmp(response, "add-relations") == 0 || strcmp(response, "r") == 0) {
		char pathbuf[1028];
		fscanf(input, "%s", pathbuf);
		Path path = CreatePath(FromCString(pathbuf));
		if (path.isAbsolut != True) {
			path = GetAbsolut(path, CopyPath(aCurrentWorkingDirectory));
		}

		ERelations newRelations = ReadInRelationsData(path);
		if (newRelations.hasData != True) {
			printf("%s\n", ErrorToString(newRelations.hasData).c_str);
			return False;
		}
		AppendRelations(&aGraph->relations, newRelations.data);
	} else if (strcmp(response, "print-person") == 0 || strcmp(response, "d") == 0) {
		int id;
		fscanf(input, "%d", &id);
		EPerson person = FindPerson(&aGraph->persons, id);
		if (person.hasData != True) {
			printf("%s\n", ErrorToString(person.hasData).c_str);
			return False;
		}

		EString string = {0};
		for (int j = 0;
			 string = PrintLineOfPersonDetailed(&person.data, j), string.hasData != OutOfRange;
			 j++) {
			if (string.hasData != True) {
				printf("%s\n", ErrorToString(string.hasData).c_str);
				return False;
			}
			printf("%s\n", string.data.c_str);
			FreeString(string.data);
		}
	} else if (strcmp(response, "play audio") == 0 || strcmp(response, "a") == 0) {
		int id;
		fscanf(input, "%d", &id);
		EPerson person = FindPerson(&aGraph->persons, id);
		if (person.hasData != True) {
			printf("%s\n", ErrorToString(person.hasData).c_str);
			return False;
		}
		if (person.data.audioPath.string.length == 0) {
			printf("no audio available.\n");
			return False;
		}
		PlayFile(person.data.audioPath);
		FreePerson(person.data);
	} else if (strcmp(response, "show images") == True || strcmp(response, "i") == True) {
		int id;
		fscanf(input, "%d", &id);
		EPerson person = FindPerson(&aGraph->persons, id);
		if (person.hasData != True) {
			printf("%s\n", ErrorToString(person.hasData).c_str);
			return False;
		}
		if (person.data.imagePaths.length == 0) {
			printf("no images available.\n");
			return False;
		}
		for (size_t i = 0; i < person.data.imagePaths.length; i++) {
			PlayFile(person.data.imagePaths.data[i]);
		}
		FreePerson(person.data);
	} else if (strcmp(response, "macro") == 0 || strcmp(response, "m") == 0) {
		char pathbuf[1028];
		fscanf(input, "%s", pathbuf);
		Path path = CreatePath(FromCString(pathbuf));
		if (path.isAbsolut != True) {
			path = GetAbsolut(path, CopyPath(aCurrentWorkingDirectory));
		}

		FILE* file = fopen(path.string.c_str, "r");
		if (!file) {
			printf("%s\n", ErrorToString(CantOpenFile).c_str);
			return False;
		}

		path = GetDirectoryPart(path);
		while (InterpreteCommand(file, aGraph, path) != EndOfFile) {
		}
		FreePath(path);
		fclose(file);
	}

	if (feof(input)) {
		return EndOfFile;
	}
	return False;
}

int
main() {
	Graph allData = {0};
	Path cwd = GetCurrentWorkingDirectory();

	Bool quit = False;
	while (quit != True) {
		printf("Enter Command: ");
		quit = InterpreteCommand(stdin, &allData, cwd);
	}

	FreePath(cwd);
	FreeGraph(allData);
	return 0;
}