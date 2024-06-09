#include "application.h"

#include "graph.h"
#include "print.h"

#include <stdio.h>

Strings
Explode(char* aString, char aDelimiter) {
	Strings result = {0};
	size_t firstChar = 0;
	for (size_t i = 0; aString[i] != '\0'; i++) {
		if (aString[i] == aDelimiter) {
			aString[i] = '\0';
			AppendString(&result, FromCString(aString + firstChar));
			firstChar = i + 1;
		}
	}
	AppendString(&result, FromCString(aString + firstChar));
	return result;
}

char*
GetLine(FILE* file) {
	const int bufLength = 20;

	int length = 0;
	char* string = malloc(bufLength * sizeof(char));
	for (char c = getc(file); c != '\n' && c != EOF; c = getc(file)) {
		length++;
		if (length % bufLength == 0) {
			string = realloc(string, (length + bufLength) * sizeof(char));
		}
		string[length - 1] = c;
	}
	if ((length + 1) % bufLength == 0) {
		string = realloc(string, (length + 1) * sizeof(char));
	}
	string[length] = '\0';
	return string;
}

ERelations
ReadInRelationsData(Path aPath) {
	ERelations result = {0};
	result.hasData = True;

	FILE* file = fopen(aPath.string.c_str, "r");
	FreePath(aPath);
	if (!file) {
		result.hasData = CantOpenFile;
		return result;
	}

	char* line = GetLine(file);
	for (; strlen(line) > 0; free(line), line = GetLine(file)) {
		Strings cells = Explode(line, ',');
		if (cells.length != 2) {
			fclose(file);
			free(line);

			result.hasData = MalformedInputFile;
			return result;
		}

		// NOTE: change this one proper format is found
		Relation relation = {0};
		relation.person1 = atoi(cells.data[0].c_str);
		relation.person2 = atoi(cells.data[1].c_str);

		AppendRelation(&result.data, relation);
	}

	fclose(file);
	free(line);
	return result;
}

EPerson
ReadInPersonalData(Path aPath) {
	EPerson result = {0};
	result.hasData = True;

	FILE* file = fopen(aPath.string.c_str, "r");
	Path dir = GetDirectoryPart(aPath);
	if (!file) {
		FreePath(dir);
		result.hasData = CantOpenFile;
		return result;
	}

	fseek(file, 0, SEEK_END);
	size_t length = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* json = malloc(length);
	if (!json) {
		fclose(file);
		FreePath(dir);
		result.hasData = InternalError;
		return result;
	}
	fread(json, 1, length, file);
	fclose(file);

	Bool isInString = False;
	Bool isInKey = True;
	String currentKey = {0};
	size_t begin = 0;
	Bool isInArray = False;
	for (size_t i = 0; i < length; i++) {
		if (json[i] == ' ' || json[i] == '\n' || json[i] == '\r' || json[i] == '\t') {
			continue;
		}
		if (json[i] == ':') {
			if (isInKey != True) {
				FreePath(dir);
				free(json);
				result.hasData = MalformedInputFile;
				return result;
			}
			isInKey = False;
			begin = i + 1;
			continue;
		}
		if (json[i] == '[') {
			isInArray = True;
			continue;
		}
		if (json[i] == '{') {
			continue;
		}
		if (json[i] == '"') {
			if (isInString == True) {
				isInString = False;

				json[i] = '\0';
				if (isInKey == True) {
					FreeString(currentKey);
					currentKey = FromCString(json + begin);
					continue;
				}

				if (strcmp(currentKey.c_str, "dateOfBirth") == 0) {
					result.data.birthDay = FromCString(json + begin);
				} else if (strcmp(currentKey.c_str, "firstNames") == 0) {
					AppendString(&result.data.firstNames, FromCString(json + begin));
				} else if (strcmp(currentKey.c_str, "lastNames") == 0) {
					AppendString(&result.data.lastNames, FromCString(json + begin));
				} else if (strcmp(currentKey.c_str, "title") == 0) {
					result.data.title = FromCString(json + begin);
				} else if (strcmp(currentKey.c_str, "titleOfNobility") == 0) {
					result.data.titleOfNobility = FromCString(json + begin);
				} else if (strcmp(currentKey.c_str, "gender") == 0) {
					result.data.gender = FromCString(json + begin);
				} else if (strcmp(currentKey.c_str, "placeOfBirth") == 0) {
					result.data.birthPlace = FromCString(json + begin);
				} else if (strcmp(currentKey.c_str, "dateOfDeath") == 0) {
					result.data.deathDay = FromCString(json + begin);
				} else if (strcmp(currentKey.c_str, "placeOfDeath") == 0) {
					result.data.deathPlace = FromCString(json + begin);
				} else if (strcmp(currentKey.c_str, "Remarks") == 0) {
					result.data.remark = FromCString(json + begin);
				} else if (strcmp(currentKey.c_str, "Audio") == 0) {
					Path path = GetAbsolut(CreatePath(FromCString(json + begin)), CopyPath(dir));
					Normalice(&path);
					result.data.audioPath = path;
				} else if (strcmp(currentKey.c_str, "Images") == 0) {
					Path path = GetAbsolut(CreatePath(FromCString(json + begin)), CopyPath(dir));
					Normalice(&path);
					AppendPath(&result.data.imagePaths, path);
				}
				continue;
			}
			begin = i + 1;
			isInString = True;
			continue;
		}
		if (isInString == True) {
			continue;
		}
		if (isInKey == True) {
			result.hasData = MalformedInputFile;
			FreePath(dir);
			free(json);
			return result;
		}
		if (json[i] == ',' && isInArray != True) {
			isInKey = True;
		} else if (json[i] == ']') {
			isInArray = False;
		}
		if (json[i] == ',' || json[i] == '}' || json[i] == ']') {
			json[i] = '\0';

			if (strcmp(currentKey.c_str, "person") == 0) {
				result.data.person = atoi(json + begin);
			}
			continue;
		}
	}
	FreeString(currentKey);
	FreePath(dir);
	free(json);
	return result;
}

Bool
PrintGeneration(Generation aGeneration) {
	int doneCount = 0;
	for (int i = 0; doneCount < aGeneration.length; i++) {
		for (size_t j = 0; j < aGeneration.length; j++) {
			EString result = PrintLineOfPersonBrief(aGeneration.data[j], i);
			if (result.hasData == OutOfRange) {
				doneCount++;
				continue;
			}
			if (result.hasData == True) {
				printf("%s", result.data.c_str);
				FreeString(result.data);
				continue;
			}
			return result.hasData;
		}
		if (doneCount < aGeneration.length) {
			printf("\n");
		}
	}
	return True;
}

// ┌───┐┌───┐
// │ 1 ││ 2 │
// └─┬─┘└─┬─┘
//   ├──┬─┴─────┐
// ┌─┴─┐│┌───┐┌─┴─┐┌───┐
// │ 3 │││ 4 ││ 5 ││ 6 │
// └─┬─┘│└─┬─┘└─┬─┘└─┬─┘
//   └──│┬─┘    │    │
//   ┌──││──────┴────┘
//   │  ││
// ┌─┴─┐││┌───┐
// │ 7 ││││ 8 │
// └─┬─┘││└─┬─┘
//   ├──││──┘
//   │  │└─┐
//   │  │  │
// ┌─┴─┐│┌─┴─┐
// │ 9 │││10 │
// └─┬─┘│└─┬─┘
//   ├──│──┘
//   │  └─┐
// ┌─┴─┐┌─┴─┐┌───┐
// │11 ││12 ││13 │
// └─┬─┘└─┬─┘└─┬─┘
//   └──┬─┤    │
//   ┌──│─┴────┘
// ┌─┴─┐│┌───┐
// │14 │││15 │
// └─┬─┘│└─┬─┘
//   ├──│──┘
//   │  └─┐
// ┌─┴─┐┌─┴─┐
// │16 ││17 │
// └─┬─┘└─┬─┘
//   ├────┘
// ┌─┴─┐
// │18 │
// └───┘
Bool
PrintGraph(Graph aGraph) {
	ArrPersonMetas data = CreateMetaData(aGraph);
	ResolveGraph(&data);
	Generations generations = CreateGenerations(&data);

	PrintGeneration(generations.data[0]);
	for (size_t i = 1; i < generations.length; i++) {
		AddProxys(&generations.data[i - 1], &generations.data[i]);
		Familys plumbingInfo = CreateFamilies(generations.data[i - 1], generations.data[i]);

		EString string = {0};
		for (int j = 0; string = PrintLineOfPlumbing(
							&generations.data[i - 1],
							&generations.data[i],
							&plumbingInfo,
							j),
				 string.hasData != OutOfRange;
			 FreeString(string.data), j++) {
			if (string.hasData != True) {
				return string.hasData;
			}
			printf("%s\n", string.data.c_str);
		}

		PrintGeneration(generations.data[i]);

		FreeString(string.data);
		FreeFamilys(plumbingInfo);
	}

	FreeGenerations(generations);
	FreeArrPersonMetas(data);
	return True;
}
