#include "json_parser.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void
ParseJson(char* aJson, JsonParsingDispatchTable aHandler) {
	if (aJson == NULL) {
		return;
	}
	JsonParsingDispatchTable* parserStack = malloc(sizeof(JsonParsingDispatchTable));
	parserStack[0] = aHandler;
	size_t parserStackDepth = 0;

	size_t isArrayBitfieldStack = 0;
	size_t bitfieldStackDepth = 0;

	size_t length = strlen(aJson);

	size_t begin = 0;
	int isInString = 0;
	int wasString = 0;
	int isInKey = 1;
	for (size_t i = 0; i < length; i++) {
		char element = aJson[i];
		// Deal with Strings
		if (element == '"') {
			if (isInString == 0) {
				begin = i + 1;
				isInString = 1;
				wasString = 1;
			} else {
				aJson[i] = '\0';
				if (isInKey == 1) {
					parserStack = realloc(
						parserStack,
						sizeof(JsonParsingDispatchTable) * (parserStackDepth + 2));
					parserStack[parserStackDepth + 1] =
						parserStack[parserStackDepth].getKeyHandler(aJson + begin);
					parserStackDepth++;
				}
				isInString = 0;
			}
			continue;
		}
		if (isInString != 0) {
			continue;
		}

		// Deal with Key
		if (isInKey == 1 && element == ':') {
			isInKey = 0;
			wasString = 0;
			begin = i + 1;
			continue;
		}

		// Deal with Value
		if (isInKey == 0 && (element == ',' || element == '}' || element == ']')) {
			aJson[i] = '\0';
			if (wasString) {
				parserStack[parserStackDepth].parseString(aJson + begin);
			} else {
				parserStack[parserStackDepth].parseInt(atoi(aJson + begin)); // NOLINT
			}
			if ((isArrayBitfieldStack & ((size_t)1 << bitfieldStackDepth)) == 0) {
				isInKey = 1;
			}
			wasString = 0;
			begin = i + 1;
		}

		// Deal with Array
		if (element == '[') {
			bitfieldStackDepth++;
			isArrayBitfieldStack |= (size_t)1 << bitfieldStackDepth;
			wasString = 0;
			begin = i + 1;
			continue;
		}

		// Deal with Object
		if (element == '{') {
			bitfieldStackDepth++;
			isArrayBitfieldStack |= (size_t) !((size_t)1 << bitfieldStackDepth);
			isInKey = 1;
			wasString = 0;
			begin = i + 1;
		}

		// Pop Parsing Strategy
		if (element == ']' || element == '}') {
			bitfieldStackDepth--;
			isInKey = 1;
			continue;
		}
	}

	free(parserStack);
}