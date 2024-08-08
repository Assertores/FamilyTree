#include "internal_types.h"
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

FullPerson*
_GetPerson(JsonPerson* self, PersonId aId, ITrace* aTrace) {
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
	FullPerson* person = newArray + self->myPersonCount;
	self->myPersons = newArray;
	memset(person, 0, sizeof(FullPerson));

	char* json = self->myPlatform->ReadFile(self->myPlatform, filePath, aTrace);
	person->data = json;
	person->folder = self->myFolders[folderIndex];

	size_t length = strlen(json);
	int isInString = 0;
	int isInKey = 1;
	char* currentKey = {0};
	size_t begin = 0;
	int isInArray = 0;
	const char** firstNameBuffer = calloc(1, sizeof(char*));
	const char** lastNameBuffer = NULL; // calloc(1, sizeof(char*));
	for (size_t i = 0; i < length; i++) {
		if (json[i] == ' ' || json[i] == '\n' || json[i] == '\r' || json[i] == '\t') {
			continue;
		}
		if (json[i] == ':') {
			if (!isInKey) {
				free(filePath);
				self->myPlatform->FreeString(self->myPlatform, json, aTrace);
				return NULL;
			}
			isInKey = 0;
			begin = i + 1;
			continue;
		}
		if (json[i] == '[') {
			isInArray = 1;
			continue;
		}
		if (json[i] == '{') {
			continue;
		}
		if (json[i] == '"') {
			if (!isInString) {
				begin = i + 1;
				isInString = 1;
				continue;
			}
			isInString = 0;

			json[i] = '\0';
			if (isInKey) {
				currentKey = json + begin;
				continue;
			}

			if (strcmp(currentKey, "DateOfBirth") == 0) {
				person->person.dateOfBirth = json + begin;
			} else if (strcmp(currentKey, "FirstNames") == 0) {
				firstNameBuffer =
					realloc(firstNameBuffer, sizeof(char*) * (person->person.firstNameCount + 1));
				if (firstNameBuffer == NULL) {
					free(filePath);
					self->myPlatform->FreeString(self->myPlatform, json, aTrace);
					return NULL;
				}
				firstNameBuffer[person->person.firstNameCount] = json + begin;
				person->person.firstNameCount++;
			} else if (strcmp(currentKey, "LastNames") == 0) {
				lastNameBuffer =
					realloc(lastNameBuffer, sizeof(char*) * (person->person.lastNameCount + 1));
				if (lastNameBuffer == NULL) {
					free(filePath);
					self->myPlatform->FreeString(self->myPlatform, json, aTrace);
					return NULL;
				}
				lastNameBuffer[person->person.lastNameCount] = json + begin;
				person->person.lastNameCount++;
			} else if (strcmp(currentKey, "Title") == 0) {
				person->person.title = json + begin;
			} else if (strcmp(currentKey, "TitleOfNobility") == 0) {
				person->person.titleOfNobility = json + begin;
			} else if (strcmp(currentKey, "Gender") == 0) {
				person->person.gender = json + begin;
			} else if (strcmp(currentKey, "PlaceOfBirth") == 0) {
				person->person.placeOfBirth = json + begin;
			} else if (strcmp(currentKey, "death") == 0) {
				person->person.dateOfDeath = json + begin;
			} else if (strcmp(currentKey, "PlaceOfDeath") == 0) {
				person->person.placeOfDeath = json + begin;
			} else if (strcmp(currentKey, "Remarks") == 0) {
				person->person.remark = json + begin;
			} else if (strcmp(currentKey, "Audio") == 0) {
				person->audioPath = json + begin;
			} else if (strcmp(currentKey, "Images") == 0) {
				person->imagePaths =
					realloc(person->imagePaths, sizeof(char*) * (person->imagePathCount + 1));
				if (person->imagePaths == NULL) {
					free(filePath);
					self->myPlatform->FreeString(self->myPlatform, json, aTrace);
					return NULL;
				}
				person->imagePaths[person->imagePathCount] = json + begin;
				person->imagePathCount++;
			}
			continue;
		}
		if (isInString) {
			continue;
		}
		if (isInKey) {
			free(filePath);
			self->myPlatform->FreeString(self->myPlatform, json, aTrace);
			return NULL;
		}
		if (json[i] == ',' && !isInArray) {
			isInKey = 1;
		} else if (json[i] == ']') {
			isInArray = 0;
		}
		if (json[i] == ',' || json[i] == '}' || json[i] == ']') {
			json[i] = '\0';

			if (strcmp(currentKey, "Id") == 0) {
				person->person.id = atoi(json + begin);
			}
			continue;
		}
	}

	person->person.firstNames = firstNameBuffer;
	person->person.lastNames = lastNameBuffer;

	self->myPersonCount++;

	free(filePath);
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
	
	FullPerson* result = _GetPerson(self, aId, aTrace);
	if(result == NULL){
		aTrace->Fail(aTrace, "Returning default constructed Person");
		return (const Person){0};
	}
	return result->person;
}

void
JsonPerson_PlayPerson(IPersonals* aThis, PersonId aId, ITrace* aTrace) {
	JsonPerson* self = (JsonPerson*)aThis;
	FullPerson* person = _GetPerson(self, aId, aTrace);

	if(person == NULL){
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
	FullPerson* person = _GetPerson(self, aId, aTrace);

	if(person == NULL){
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

		size_t numberbegin = 0;
		size_t numberlength = 0;
		for (size_t j = 0;; j++) {
			if (file[j] == '\0') {
				break;
			}
			if (strncmp(file + j, "Id", 6) == 0) {
				j += 6;
				numberbegin = j;
				continue;
			}
			if (numberbegin == 0) {
				continue;
			}
			if (!isdigit(file[j])) {
				if (numberlength == 0) {
					numberbegin++;
					continue;
				}
				break;
			}
			numberlength++;
		}
		if (numberlength == 0) {
			continue;
		}
		file[numberbegin + numberlength + 1] = '\0';
		self->myIds[i] = atoi(file + numberbegin);

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