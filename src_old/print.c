#include "print.h"

#include <stdio.h>

#ifdef _WIN32
#define UR "\xda"
#define LOR "\xc1"
#define LR "\xc4"
#define LU "\xbf"
#define OU "\xb3"
#define OR "\xc0"
#define LUR "\xc2"
#define LO "\xd9"
#define ORU "\xc3"
#define ORUL "\xc5"
#define MALE "\x0b"
#define FEMALE "\x0c"
#else
#define UR "┌"
#define LOR "┴"
#define LR "─"
#define LU "┐"
#define OU "│"
#define OR "└"
#define LUR "┬"
#define LO "┘"
#define ORU "├"
#define ORUL "┼"
#define MALE "♂"
#define FEMALE "♀"
#endif

// ┌1───────────────────┴────────────────────┐
// │Prof. Dr. Philippa R. P. von und zu Perry│
// │♂ │ * 04.05.1996 │ + 11.11.2021          │
// └────────────────────┬────────────────────┘
// ┌2──────────┴───────────┐
// │ Katy S. V. J.  Santos │
// │♀ │ * 01.01.1986 │ +   │
// └───────────┬───────────┘
// ┌3─────────┴──────────┐
// │ Pippa  Ross         │
// │n │ * 04.05.1996 │ + │
// └──────────┬──────────┘
// ┌4──────────┴───────────┐
// │ Constance F.  Houghton│
// │♀ │ * 04.05.1996 │ +   │
// └───────────┬───────────┘
EString
PrintLineOfPersonBrief(const PersonMeta* aPerson, unsigned int aLineNumber) {
	if (aLineNumber > 3) {
		EString result = {0};
		result.hasData = OutOfRange;
		return result;
	}
	if (aPerson->isProxy == True) {
		EString result = {0};
		result.data = FromCString(OU);
		result.hasData = True;
		return result;
	}

	EString result = {0};
	if (aLineNumber == 0) {
		result.data = ConcatLiteral(result.data, UR);
		char id[12];
		sprintf(id, "%d", aPerson->person->person);
		result.data = Concatinate(result.data, FromCString(id));
		for (int i = strlen(id); i < aPerson->briefWidth / 2 - 1; i++) {
			result.data = ConcatLiteral(result.data, LR);
		}
		result.data = ConcatLiteral(result.data, LOR);
		for (int i = 0; i < aPerson->briefWidth / 2 - 1; i++) {
			result.data = ConcatLiteral(result.data, LR);
		}
		result.data = ConcatLiteral(result.data, LU);
	} else if (aLineNumber == 1) {
		result.data = ConcatLiteral(result.data, OU);
		// NOTE: we don't want to steal ownership of aPerson.
		result.data = ConcatConst(result.data, aPerson->person->title);
		result.data = ConcatLiteral(result.data, " ");
		result.data = ConcatConst(result.data, aPerson->person->firstNames.data[0]);
		result.data = ConcatLiteral(result.data, " ");
		for (size_t i = 1; i < aPerson->person->firstNames.length; i++) {
			if (aPerson->person->firstNames.data[i].length > 0) {
				char abreveation[2] = {0};
				abreveation[0] = aPerson->person->firstNames.data[i].c_str[0];
				result.data = ConcatLiteral(result.data, abreveation);
			}
			result.data = ConcatLiteral(result.data, ". ");
		}
		result.data = ConcatConst(result.data, aPerson->person->titleOfNobility);
		result.data = ConcatLiteral(result.data, " ");
		result.data = ConcatConst(result.data, aPerson->person->lastNames.data[0]);
		for (size_t i = result.data.length + 1; i < aPerson->briefWidth; i++) {
			result.data = ConcatLiteral(result.data, " ");
		}
		result.data = ConcatLiteral(result.data, OU);
	} else if (aLineNumber == 2) {
		result.data = ConcatLiteral(result.data, OU);
		if (aPerson->person->gender.length > 0) {
			if (strcmp(aPerson->person->gender.c_str, "male") == 0) {
				result.data = ConcatLiteral(result.data, MALE);
			} else if (strcmp(aPerson->person->gender.c_str, "female") == 0) {
				result.data = ConcatLiteral(result.data, FEMALE);
			} else {
				char abreveation[2] = {0};
				abreveation[0] = aPerson->person->gender.c_str[0];
				result.data = ConcatLiteral(result.data, abreveation);
			}
		} else {
			result.data = ConcatLiteral(result.data, "?");
		}
		result.data = ConcatLiteral(result.data, " " OU " * ");
		result.data = ConcatConst(result.data, aPerson->person->birthDay);
		result.data = ConcatLiteral(result.data, " " OU " + ");
		result.data = ConcatConst(result.data, aPerson->person->deathDay);
		for (size_t i = result.data.length + 1; i < aPerson->briefWidth; i++) {
			result.data = ConcatLiteral(result.data, " ");
		}
		result.data = ConcatLiteral(result.data, OU);
	} else if (aLineNumber == 3) {
		result.data = ConcatLiteral(result.data, OR);
		for (int i = 0; i < aPerson->briefWidth / 2 - 1; i++) {
			result.data = ConcatLiteral(result.data, LR);
		}
		result.data = ConcatLiteral(result.data, LUR);
		for (int i = 0; i < aPerson->briefWidth / 2 - 1; i++) {
			result.data = ConcatLiteral(result.data, LR);
		}
		result.data = ConcatLiteral(result.data, LO);
	}
	result.hasData = True;
	return result;
}

int
CharCountToPersonCenter(const Generation* aGeneration, const PersonMeta* aKey) {
	int result = 0;
	for (size_t i = 0; i < aGeneration->length; i++) {
		if (EqualPersonMeta(aKey, aGeneration->data[i]) == True) {
			result += (aGeneration->data[i]->briefWidth / 2) + 1;
			break;
		}
		result += aGeneration->data[i]->briefWidth;
	}
	return result;
}

Bool
Findint(const ints* aSet, int aKey) {
	for (size_t i = 0; i < aSet->length; i++) {
		if (aSet->data[i] == aKey) {
			return True;
		}
	}
	return NotFound;
}

// ┌1───────────────────┴────────────────────┐┌2──────────┴───────────┐┌3─────────┴──────────┐│
// │Prof. Dr. Philippa R. P. von und zu Perry││ Katy S. V. J.  Santos ││ Pippa  Ross         ││
// │♂ │ * 04.05.1996 │ + 11.11.2021          ││♀ │ * 01.01.1986 │ +   ││n │ * 04.05.1996 │ + ││
// └────────────────────┬────────────────────┘└───────────┬───────────┘└──────────┬──────────┘│
//                      │                                 │                       │           │
//                      │              ┬──────────────────│───────────────────────┴───────────┴
//             ┬────────┴──────────────│──────────────────┴
//             │                       │
// ┌4──────────┴───────────┐┌5─────────┴──────────┐
// │ Constance F.  Houghton││ Bethany H.  Barnes  │
// │♀ │ * 04.05.1996 │ +   ││♀ │ * 01.01.1986 │ + │
// └───────────┬───────────┘└──────────┬──────────┘
EString
PrintLineOfPlumbing(
	const Generation* aTopGeneration,
	const Generation* aBottomGeneration,
	const Familys* aFamilys,
	unsigned int aLineNumber) {
	if (aLineNumber >= 2 + aFamilys->length) {
		EString result = {0};
		result.hasData = OutOfRange;
		return result;
	}

	if (aLineNumber == 0 || aLineNumber == 1 + aFamilys->length) {
		const Generation* aTargetGeneration;
		if (aLineNumber == 0) {
			aTargetGeneration = aTopGeneration;
		} else {
			aTargetGeneration = aBottomGeneration;
		}

		EString result = {0};
		for (size_t i = 0; i < aTargetGeneration->length; i++) {
			if (aTargetGeneration->data[i]->isProxy == True) {
				result.data = ConcatLiteral(result.data, OU);
				continue;
			}

			for (int j = 0; j < aTargetGeneration->data[i]->briefWidth / 2; j++) {
				result.data = ConcatLiteral(result.data, " ");
			}
			result.data = ConcatLiteral(result.data, OU);
			for (int j = 0; j < aTargetGeneration->data[i]->briefWidth / 2; j++) {
				result.data = ConcatLiteral(result.data, " ");
			}
		}

		result.hasData = True;
		return result;
	}

	ints passTroughs = {0};
	for (size_t i = 0; i < aLineNumber - 1; i++) {
		for (size_t j = 0; j < aFamilys->data[i].childs.length; j++) {
			Appendint(
				&passTroughs,
				CharCountToPersonCenter(aBottomGeneration, aFamilys->data[i].childs.data[j]));
		}
	}
	for (size_t i = aLineNumber; i < aFamilys->length; i++) {
		for (size_t j = 0; j < aFamilys->data[i].parents.length; j++) {
			Appendint(
				&passTroughs,
				CharCountToPersonCenter(aTopGeneration, aFamilys->data[i].parents.data[j]));
		}
	}

	ints parents = {0};
	for (size_t i = 0; i < aFamilys->data[aLineNumber - 1].parents.length; i++) {
		Appendint(
			&parents,
			CharCountToPersonCenter(
				aTopGeneration,
				aFamilys->data[aLineNumber - 1].parents.data[i]));
	}

	ints childs = {0};
	for (size_t i = 0; i < aFamilys->data[aLineNumber - 1].childs.length; i++) {
		Appendint(
			&childs,
			CharCountToPersonCenter(
				aBottomGeneration,
				aFamilys->data[aLineNumber - 1].childs.data[i]));
	}

	int min = parents.data[0];
	int max = parents.data[0];
	for (size_t i = 1; i < parents.length; i++) {
		min = parents.data[i] < min ? parents.data[i] : min;
		max = parents.data[i] > max ? parents.data[i] : max;
	}
	for (size_t i = 0; i < childs.length; i++) {
		min = childs.data[i] < min ? childs.data[i] : min;
		max = childs.data[i] > max ? childs.data[i] : max;
	}
	int lineLength = max;
	for (size_t i = 0; i < passTroughs.length; i++) {
		lineLength = passTroughs.data[i] > lineLength ? passTroughs.data[i] : lineLength;
	}

	EString result = {0};
	for (int i = 1; i <= lineLength; i++) {
		Bool isParent = Findint(&parents, i);
		Bool isChild = Findint(&childs, i);
		if (isParent == True && isChild == True) {
			result.data = ConcatLiteral(result.data, ORUL);
			continue;
		}
		if (isParent == True) {
			result.data = ConcatLiteral(result.data, LOR);
			continue;
		}
		if (isChild == True) {
			result.data = ConcatLiteral(result.data, LUR);
			continue;
		}
		if (Findint(&passTroughs, i) == True) {
			result.data = ConcatLiteral(result.data, OU);
			continue;
		}
		if (i < min || i > max) {
			result.data = ConcatLiteral(result.data, " ");
			continue;
		}
		result.data = ConcatLiteral(result.data, LR);
	}
	result.hasData = True;
	return result;
}

EString
PrintLineOfPersonDetailed(const Person* aPerson, unsigned int aLineNumber) {
	EString result = {0};
	result.hasData = OutOfRange;

	int lineCount = 0;
	if (aLineNumber == lineCount) {
		result.hasData = True;
		result.data = ConcatLiteral(result.data, "Person id:    ");
		char id[12];
		sprintf(id, "%d", aPerson->person);
		result.data = ConcatLiteral(result.data, id);
	}
	lineCount++;
	if (aLineNumber == lineCount) {
		result.hasData = True;
		result.data = ConcatLiteral(result.data, "Name:         ");
		result.data = ConcatConst(result.data, aPerson->title);
		result.data = ConcatLiteral(result.data, " ");
		for (size_t i = 0; i < aPerson->firstNames.length; i++) {
			if (aPerson->firstNames.data[i].length > 0) {
				result.data = ConcatConst(result.data, aPerson->firstNames.data[i]);
				result.data = ConcatLiteral(result.data, " ");
			}
		}
		result.data = ConcatConst(result.data, aPerson->titleOfNobility);
		result.data = ConcatLiteral(result.data, " ");
		for (size_t i = 0; i < aPerson->lastNames.length; i++) {
			if (aPerson->lastNames.data[i].length > 0) {
				result.data = ConcatConst(result.data, aPerson->lastNames.data[i]);
				result.data = ConcatLiteral(result.data, " ");
			}
		}
	}
	if (aPerson->gender.length > 0) {
		lineCount++;
		if (aLineNumber == lineCount) {
			result.hasData = True;
			result.data = ConcatLiteral(result.data, "Gender:       ");
			result.data = ConcatConst(result.data, aPerson->gender);
		}
	}

	if (aPerson->birthDay.length > 0) {
		lineCount++;
		if (aLineNumber == lineCount) {
			result.hasData = True;
			result.data = ConcatLiteral(result.data, "Day Of birth: ");
			result.data = ConcatConst(result.data, aPerson->birthDay);
			if (aPerson->birthPlace.length > 0) {
				result.data = ConcatLiteral(result.data, "(");
				result.data = ConcatConst(result.data, aPerson->birthPlace);
				result.data = ConcatLiteral(result.data, ")");
			}
		}
	}
	if (aPerson->deathDay.length > 0) {
		lineCount++;
		if (aLineNumber == lineCount) {
			result.hasData = True;
			result.data = ConcatLiteral(result.data, "Day of death: ");
			result.data = ConcatConst(result.data, aPerson->deathDay);
			if (aPerson->deathPlace.length > 0) {
				result.data = ConcatLiteral(result.data, "(");
				result.data = ConcatConst(result.data, aPerson->deathPlace);
				result.data = ConcatLiteral(result.data, ")");
			}
		}
	}

	if (aPerson->audioPath.string.length > 0 || aPerson->imagePaths.length > 0) {
		lineCount++;
		if (aLineNumber == lineCount) {
			result.hasData = True;
		}
	}

	if (aPerson->audioPath.string.length > 0) {
		lineCount++;
		if (aLineNumber == lineCount) {
			result.hasData = True;
			result.data = ConcatLiteral(result.data, "Audio available");
		}
	}

	if (aPerson->imagePaths.length > 0) {
		lineCount++;
		if (aLineNumber == lineCount) {
			result.hasData = True;
			result.data = ConcatLiteral(result.data, "Images available");
		}
	}

	if (aPerson->remark.length > 0) {
		lineCount++;
		if (aLineNumber == lineCount) {
			result.hasData = True;
		}
		lineCount++;
		if (aLineNumber == lineCount) {
			result.hasData = True;
			result.data = ConcatLiteral(result.data, "Remarks:");
		}
		lineCount++;
		if (aLineNumber == lineCount) {
			result.hasData = True;
			// TODO: multiline remarks?
			result.data = ConcatConst(result.data, aPerson->remark);
		}
	}
	return result;
}