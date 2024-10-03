#include "internal_types.h"
#include "json_parser.h"
#include "patch.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	Person person;
	const char* folder;
	const char* audioPath;
	size_t imagePathCount;
	const char** imagePaths;

	char* data;
} FullPerson;

typedef struct {
	IPersonals interface;

	char* myPath;
	IPlatform* myPlatform;

	size_t myFolderCount;
	char** myFolders;
	PersonId* myIds;

	size_t myPersonCount;
	FullPerson* myPersons;
} JsonPerson;

FullPerson* thePerson = NULL;
const char** theFirstNameBuffer = NULL;
const char** theLastNameBuffer = NULL;
const char** theProfessions = NULL;
PersonId theId = 0;

void
PrivOnId(int aValue) {
	theId = aValue;
	if (thePerson == NULL) {
		return;
	}
	thePerson->person.id = aValue;
}

void
PrivOnTitle(const char* aValue) {
	if (thePerson == NULL) {
		// TODO: error handling
		return;
	}
	thePerson->person.title = aValue;
}

void
PrivOnFirstName(const char* aValue) {
	if (thePerson == NULL) {
		// TODO: error handling
		return;
	}
	theFirstNameBuffer =
		realloc(theFirstNameBuffer, sizeof(char*) * (thePerson->person.firstNameCount + 1));
	if (theFirstNameBuffer == NULL) {
		// TODO: error handling
		return;
	}
	theFirstNameBuffer[thePerson->person.firstNameCount] = aValue;
	thePerson->person.firstNameCount++;
}

void
PrivOnTitleOfNobility(const char* aValue) {
	if (thePerson == NULL) {
		// TODO: error handling
		return;
	}
	thePerson->person.titleOfNobility = aValue;
}

void
PrivOnLastName(const char* aValue) {
	if (thePerson == NULL) {
		// TODO: error handling
		return;
	}
	theLastNameBuffer =
		realloc(theLastNameBuffer, sizeof(char*) * (thePerson->person.lastNameCount + 1));
	if (theLastNameBuffer == NULL) {
		// TODO: error handling
		return;
	}
	theLastNameBuffer[thePerson->person.lastNameCount] = aValue;
	thePerson->person.lastNameCount++;
}

void
PrivOnGender(const char* aValue) {
	if (thePerson == NULL) {
		// TODO: error handling
		return;
	}
	thePerson->person.gender = aValue;
}

void
PrivOnDateOfBirth(const char* aValue) {
	if (thePerson == NULL) {
		// TODO: error handling
		return;
	}
	thePerson->person.dateOfBirth = aValue;
}

void
PrivOnPlaceOfBirth(const char* aValue) {
	if (thePerson == NULL) {
		// TODO: error handling
		return;
	}
	thePerson->person.placeOfBirth = aValue;
}

void
PrivOnDateOfDeath(const char* aValue) {
	if (thePerson == NULL) {
		// TODO: error handling
		return;
	}
	thePerson->person.dateOfDeath = aValue;
}

void
PrivOnPlaceOfDeath(const char* aValue) {
	if (thePerson == NULL) {
		// TODO: error handling
		return;
	}
	thePerson->person.placeOfDeath = aValue;
}

void
PrivOnAudio(const char* aValue) {
	if (thePerson == NULL) {
		// TODO: error handling
		return;
	}
	thePerson->audioPath = aValue;
}

void
PrivOnImage(const char* aValue) {
	if (thePerson == NULL) {
		// TODO: error handling
		return;
	}
	thePerson->imagePaths =
		realloc(thePerson->imagePaths, sizeof(char*) * (thePerson->imagePathCount + 1));
	if (thePerson->imagePaths == NULL) {
		// TODO: error handling
		return;
	}
	thePerson->imagePaths[thePerson->imagePathCount] = aValue;
	thePerson->imagePathCount++;
}

void
PrivOnRemarks(const char* aValue) {
	if (thePerson == NULL) {
		// TODO: error handling
		return;
	}
	thePerson->person.remark = aValue;
}

void
PrivOnProfession(const char* aValue) {
	if (thePerson == NULL) {
		// TODO: error handling
		return;
	}
	theProfessions =
		realloc(theProfessions, sizeof(char*) * (thePerson->person.professionCount + 1));
	if (theProfessions == NULL) {
		// TODO: error handling
		return;
	}
	theProfessions[thePerson->person.professionCount] = aValue;
	thePerson->person.professionCount++;
}

void
PrivNoOpInt(int aValue) {}
void
PrivNoOpString(const char* aValue) {}

void
PrivOnName(const char* aValue) {
	thePerson->person.placeOfResidences[thePerson->person.placeOfResidenceCount - 1].name = aValue;
}

void
PrivOnStartDate(const char* aValue) {
	thePerson->person.placeOfResidences[thePerson->person.placeOfResidenceCount - 1].startDate =
		aValue;
}

void
PrivOnEndData(const char* aValue) {
	thePerson->person.placeOfResidences[thePerson->person.placeOfResidenceCount - 1].endDate =
		aValue;
}

JsonParsingDispatchTable
PrivOnResidence(const char* aKey) {
	JsonParsingDispatchTable table;
	table.getKeyHandler = PrivOnResidence;
	table.parseInt = PrivNoOpInt;
	table.parseString = PrivNoOpString;

	if (strcmp(aKey, "Name") == 0) {
		table.parseString = PrivOnName;
	} else if (strcmp(aKey, "StartDate") == 0) {
		table.parseString = PrivOnStartDate;
	} else if (strcmp(aKey, "EndData") == 0) {
		table.parseString = PrivOnEndData;
	}

	return table;
}

JsonParsingDispatchTable
PrivOnKey(const char* aKey) {
	JsonParsingDispatchTable table;
	table.getKeyHandler = PrivOnKey;
	table.parseInt = PrivNoOpInt;
	table.parseString = PrivNoOpString;

	if (strcmp(aKey, "Id") == 0) {
		table.parseInt = PrivOnId;
	} else if (strcmp(aKey, "FirstNames") == 0) {
		table.parseString = PrivOnFirstName;
	} else if (strcmp(aKey, "LastNames") == 0) {
		table.parseString = PrivOnLastName;
	} else if (strcmp(aKey, "Title") == 0) {
		table.parseString = PrivOnTitle;
	} else if (strcmp(aKey, "TitleOfNobility") == 0) {
		table.parseString = PrivOnTitleOfNobility;
	} else if (strcmp(aKey, "Gender") == 0) {
		table.parseString = PrivOnGender;
	} else if (strcmp(aKey, "DateOfBirth") == 0) {
		table.parseString = PrivOnDateOfBirth;
	} else if (strcmp(aKey, "PlaceOfBirth") == 0) {
		table.parseString = PrivOnPlaceOfBirth;
	} else if (strcmp(aKey, "DateOfDeath") == 0) {
		table.parseString = PrivOnDateOfDeath;
	} else if (strcmp(aKey, "PlaceOfDeath") == 0) {
		table.parseString = PrivOnPlaceOfDeath;
	} else if (strcmp(aKey, "Remarks") == 0) {
		table.parseString = PrivOnRemarks;
	} else if (strcmp(aKey, "Audio") == 0) {
		table.parseString = PrivOnAudio;
	} else if (strcmp(aKey, "Images") == 0) {
		table.parseString = PrivOnImage;
	} else if (strcmp(aKey, "Profession") == 0) {
		table.parseString = PrivOnProfession;
	} else if (strcmp(aKey, "PlaceOfResidence") == 0) {
		thePerson->person.placeOfResidenceCount++;
		thePerson->person.placeOfResidences = realloc(
			thePerson->person.placeOfResidences,
			sizeof(Residence) * thePerson->person.placeOfResidenceCount);
		memset(
			&thePerson->person.placeOfResidences[thePerson->person.placeOfResidenceCount - 1],
			0,
			sizeof(Residence));

		table.getKeyHandler = PrivOnResidence;
	}
	return table;
}

JsonParsingDispatchTable
PrivOnlyId(const char* aKey) {
	JsonParsingDispatchTable table;
	table.getKeyHandler = PrivOnlyId;
	table.parseInt = PrivNoOpInt;
	table.parseString = PrivNoOpString;
	if (strcmp(aKey, "Id") == 0) {
		table.parseInt = PrivOnId;
	}
	return table;
}

FullPerson*
PrivGetPerson(JsonPerson* self, PersonId aId, ITrace* aTrace) {
	for (size_t i = 0; i < self->myPersonCount; i++) {
		if (self->myPersons[i].person.id == aId) {
			return self->myPersons + i;
		}
	}

	size_t folderIndex = 0;
	for (size_t i = 0; i < self->myFolderCount; i++) {
		if (self->myIds[i] == aId) {
			folderIndex = i;
			break;
		}
	}

	size_t pathLength = strlen(self->myPath);
	size_t folderLength = strlen(self->myFolders[folderIndex]);
	char* filePath = malloc(pathLength + 1 + folderLength + 11);
	strcpy_s(filePath, pathLength + 1, self->myPath);
	filePath[pathLength] = '/';
	strcpy_s(filePath + pathLength + 1, folderLength + 1, self->myFolders[folderIndex]);
	strcpy_s(filePath + pathLength + 1 + folderLength, 11, "/data.json");

	FullPerson* newArray = realloc(self->myPersons, (self->myPersonCount + 1) * sizeof(FullPerson));
	if (newArray == NULL) {
		// TODO: error handling
		free(filePath);
		return NULL;
	}
	thePerson = newArray + self->myPersonCount;
	self->myPersons = newArray;
	memset(thePerson, 0, sizeof(FullPerson));

	char* json = self->myPlatform->ReadFile(self->myPlatform, filePath, aTrace);
	thePerson->data = json;
	thePerson->folder = self->myFolders[folderIndex];

	JsonParsingDispatchTable table;
	table.getKeyHandler = PrivOnKey;
	table.parseInt = PrivNoOpInt;
	table.parseString = PrivNoOpString;
	ParseJson(json, table);
	FullPerson* person = thePerson;

	person->person.firstNames = theFirstNameBuffer;
	person->person.lastNames = theLastNameBuffer;
	person->person.professions = theProfessions;
	self->myPersonCount++;

	free(filePath);
	thePerson = NULL;
	theFirstNameBuffer = NULL;
	theLastNameBuffer = NULL;
	return person;
}

IPersonals*
JsonPerson_Copy(IPersonals* aThis, ITrace* aTrace) {
	return aThis;
}

size_t
JsonPerson_GetAllIdsCount(IPersonals* aThis, ITrace* aTrace) {
	JsonPerson* self = (JsonPerson*)aThis;

	return self->myFolderCount;
}

void
JsonPerson_GetAllIds(IPersonals* aThis, PersonId* aOutId, ITrace* aTrace) {
	JsonPerson* self = (JsonPerson*)aThis;

	for (size_t i = 0; i < self->myFolderCount; i++) {
		aOutId[i] = self->myIds[i];
	}
}

Person
JsonPerson_GetPerson(IPersonals* aThis, PersonId aId, ITrace* aTrace) {
	JsonPerson* self = (JsonPerson*)aThis;

	FullPerson* result = PrivGetPerson(self, aId, aTrace);
	if (result == NULL) {
		aTrace->Fail(aTrace, "Returning default constructed Person");
		return (const Person){0};
	}
	return result->person;
}

void
JsonPerson_PlayPerson(IPersonals* aThis, PersonId aId, ITrace* aTrace) {
	JsonPerson* self = (JsonPerson*)aThis;
	FullPerson* person = PrivGetPerson(self, aId, aTrace);

	if (person == NULL) {
		aTrace->Fail(aTrace, "unable to find person");
		return;
	}

	size_t pathLength = strlen(self->myPath);
	size_t folderLength = strlen(person->folder);
	size_t fileLength = strlen(person->audioPath);

	char* path = malloc(pathLength + 1 + folderLength + 1 + fileLength + 1);
	strcpy_s(path, pathLength + 1, self->myPath);
	path[pathLength] = '/';
	strcpy_s(path + pathLength + 1, folderLength + 1, person->folder);
	path[pathLength + 1 + folderLength] = '/';
	strcpy_s(path + pathLength + 1 + folderLength + 1, fileLength + 1, person->audioPath);

	self->myPlatform->OpenAudio(self->myPlatform, path, aTrace);
	free(path);
}

void
JsonPerson_ShowImages(IPersonals* aThis, PersonId aId, ITrace* aTrace) {
	JsonPerson* self = (JsonPerson*)aThis;
	FullPerson* person = PrivGetPerson(self, aId, aTrace);

	if (person == NULL) {
		aTrace->Fail(aTrace, "unable to find person");
		return;
	}

	size_t pathLength = strlen(self->myPath);
	size_t folderLength = strlen(person->folder);

	size_t longestFileLength = 0;
	for (size_t i = 0; i < person->imagePathCount; i++) {
		size_t length = strlen(person->imagePaths[i]);
		if (length > longestFileLength) {
			longestFileLength = length;
		}
	}
	longestFileLength++;

	char* path = malloc(pathLength + 1 + folderLength + 1 + longestFileLength);
	strcpy_s(path, pathLength + 1, self->myPath);
	path[pathLength] = '/';
	strcpy_s(path + pathLength + 1, folderLength + 1, person->folder);
	path[pathLength + 1 + folderLength] = '/';

	for (size_t i = 0; i < person->imagePathCount; i++) {
		memset(path + pathLength + 1 + folderLength + 1, '\0', longestFileLength);
		strcpy_s(
			path + pathLength + 1 + folderLength + 1,
			longestFileLength,
			person->imagePaths[i]);
		self->myPlatform->OpenImage(self->myPlatform, path, aTrace);
	}

	free(path);
}

void
JsonPerson_Free(IPersonals* aThis, ITrace* aTrace) {
	JsonPerson* self = (JsonPerson*)aThis;

	if (self->myFolderCount > 0) {
		self->myPlatform->FreeString(self->myPlatform, self->myFolders[0], aTrace);
	}
	for (size_t i = 0; i < self->myPersonCount; i++) {
		free(self->myPersons[i].imagePaths);
		free((void*)self->myPersons[i].person.firstNames);
		free((void*)self->myPersons[i].person.lastNames);
		self->myPlatform->FreeString(self->myPlatform, self->myPersons[i].data, aTrace);
	}
	self->myPlatform->Free(self->myPlatform, aTrace);

	free(self->myPath);
	free(self->myFolders);
	free(self->myIds);
	free(self->myPersons);
	free(self);
}

void
ResetFolders(JsonPerson* self, ITrace* aTrace) {
	if (self->myFolderCount > 0) {
		self->myPlatform->FreeString(self->myPlatform, self->myFolders[0], aTrace);
	}
	for (size_t i = 0; i < self->myPersonCount; i++) {
		free(self->myPersons[i].imagePaths);
		free((void*)self->myPersons[i].person.firstNames);
		free((void*)self->myPersons[i].person.lastNames);
		self->myPlatform->FreeString(self->myPlatform, self->myPersons[i].data, aTrace);
	}
	self->myPersonCount = 0;
	self->myPersons = realloc(self->myPersons, sizeof(FullPerson));

	char* folders = self->myPlatform->GetFolders(self->myPlatform, self->myPath, aTrace);

	int count = 0;
	for (size_t i = 0;; i++) {
		if (folders[i] == '\0') {
			if (folders[i + 1] == '\0') {
				break;
			}
			count++;
		}
	}
	if (count == 0) {
		// TODO: error handling
		return;
	}
	count++;
	self->myFolderCount = count;

	self->myFolders = realloc(self->myFolders, count * sizeof(char**));
	self->myFolders[0] = folders;
	count = 0;
	for (size_t i = 0;; i++) {
		if (folders[i] == '\0') {
			if (folders[i + 1] == '\0') {
				break;
			}
			count++;
			self->myFolders[count] = folders + i + 1;
		}
	}
}

void
ResetIds(JsonPerson* self, ITrace* aTrace) {
	self->myIds = realloc(self->myIds, self->myFolderCount * sizeof(PersonId));

	size_t longestFolderName = 0;
	for (size_t i = 0; i < self->myFolderCount; i++) {
		size_t length = strlen(self->myFolders[i]);
		if (length > longestFolderName) {
			longestFolderName = length;
		}
	}
	longestFolderName++;

	size_t rootLength = strlen(self->myPath);
	char* path = malloc(rootLength + 1 + longestFolderName + 10);
	strcpy_s(path, rootLength + 1, self->myPath);
	path[rootLength] = '/';

	for (size_t i = 0; i < self->myFolderCount; i++) {
		memset(path + rootLength + 1, '\0', longestFolderName);
		strcpy_s(path + rootLength + 1, longestFolderName + 1, self->myFolders[i]);
		strcpy_s(path + rootLength + 1 + strlen(self->myFolders[i]), 11, "/data.json");
		char* file = self->myPlatform->ReadFile(self->myPlatform, path, aTrace);

		JsonParsingDispatchTable table;
		table.getKeyHandler = PrivOnlyId;
		table.parseInt = PrivNoOpInt;
		table.parseString = PrivNoOpString;
		theId = 0;
		ParseJson(file, table);
		self->myIds[i] = theId;

		self->myPlatform->FreeString(self->myPlatform, file, aTrace);
	}
	free(path);
}

IPersonals*
CreateJSONPerson(const char* aPath, IPlatform* aPlatform, ITrace* aTrace) {
	ITrace* trace = aTrace->CreateSubTrace(aTrace, "Construct JSON Persons");
	trace->AddEvent(trace, "Allocate memory");
	JsonPerson* result = calloc(1, sizeof(JsonPerson));

	trace->AddEvent(trace, "Set Dispatch Table");
	result->interface.Copy = JsonPerson_Copy;
	result->interface.GetAllIdsCount = JsonPerson_GetAllIdsCount;
	result->interface.GetAllIds = JsonPerson_GetAllIds;
	result->interface.GetPerson = JsonPerson_GetPerson;
	result->interface.PlayPerson = JsonPerson_PlayPerson;
	result->interface.ShowImages = JsonPerson_ShowImages;
	result->interface.Free = JsonPerson_Free;

	trace->AddEvent(trace, "Copy string to get in controle of livetime");
	size_t length = strlen(aPath);
	char* path = malloc(length + 1);
	strcpy_s(path, length + 1, aPath);
	result->myPath = path;

	result->myPlatform = aPlatform;

	ResetFolders(result, trace);
	ResetIds(result, trace);

	return &result->interface;
}