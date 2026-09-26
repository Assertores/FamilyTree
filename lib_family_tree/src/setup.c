#include "setup.h"

#include "family_tree/api.h"
#include "platform.h"

#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void
StoreString(Utf8_String aString, Internals* aInternals) {
	void* new = realloc(
		(void*)aInternals->allocatedStrings,
		(aInternals->allocatedStringCount + 1) * sizeof(Utf8_String));
	if (new == NULL) {
		// TODO: deal with failed realloc
		return;
	}

	aInternals->allocatedStrings = (Utf8_String*)new;
	aInternals->allocatedStrings[aInternals->allocatedStringCount] = aString;
	aInternals->allocatedStringCount++;
}

void
DoFolders(ContextBuilder* aPreviousStep, DebuggingData* aDebug) {
	Utf8_String string = GetFolders(aPreviousStep->qFoldername);
	StoreString(string, aPreviousStep->data);

	if (string[0] == '\0' && string[1] == '\0') {
		// TODO: error handling
		return;
	}

	int count = 1;
	for (size_t i = 0;; i++) {
		if (string[i] == '\0') {
			if (string[i + 1] == '\0') {
				break;
			}
			count++;
		}
	}
	aPreviousStep->arrayCount = count;
	aPreviousStep->aStrings = (Utf8_String*)calloc(count, sizeof(Utf8_String));

	aPreviousStep->aStrings[0] = string;
	count = 0;
	for (size_t i = 0;; i++) {
		if (string[i] == '\0') {
			if (string[i + 1] == '\0') {
				break;
			}
			count++;
			aPreviousStep->aStrings[count] = string + i + 1;
		}
	}
}

void
DoFile(ContextBuilder* aPreviousStep, DebuggingData* aDebug) {
	Utf8_String string = ReadFile(aPreviousStep->qFilename);

	Context* context = ((Internals*)aPreviousStep->data)->context;
	
	void* new = realloc(
		(void*)context->allocatedStrings,
		(context->allocatedStringCount + 1) * sizeof(Utf8_String));
	if (new == NULL) {
		// TODO: deal with failed realloc
		return;
	}

	context->allocatedStrings = (Utf8_String*)new;
	context->allocatedStrings[context->allocatedStringCount] = string;
	context->allocatedStringCount++;

	aPreviousStep->aString = string;
}

void
ParsePerson(ContextBuilder* aPreviousStep, DebuggingData* aDebug) {}

void
ParseRelation(ContextBuilder* aPreviousStep, DebuggingData* aDebug) {}

Context*
CreateContext(Internals* aInternals) {
	// TODO: run algorythems
	return aInternals->context;
}

void FreeInternals(Internals* aInternals){
	for(size_t i = 0; i < aInternals->allocatedStringCount; i++){
		free((void*)aInternals->allocatedStrings[i]);
	}
	free((void*)aInternals->allocatedStrings);
	
	for(size_t i = 0; i < aInternals->foldernameCount; i++){
		free((void*)aInternals->foldernames[i]);
	}
	free((void*)aInternals->foldernames);
}

ContextBuilder
DoStart(Utf8_String aPath, DebuggingData* aDebug) {
	ContextBuilder firstStep;

	Internals* internals = calloc(1, sizeof(Internals));
	internals->rootPath = aPath;
	internals->rootPathStrlen = strlen(aPath);
	internals->isRelations = 1;
	internals->context = calloc(1, sizeof(Context));
	firstStep.data = internals;

	size_t newLength = internals->rootPathStrlen + sizeof("relations.csv");
	char* path = calloc(newLength, sizeof(char));
	StoreString(path, internals);

	firstStep.question = QuestionType_ContentOfFile;
	strcpy(path, internals->rootPath);
	strcpy(path + internals->rootPathStrlen, "relations.csv");
	firstStep.qFilename = path;
	return firstStep;
}

ContextBuilder
DoContinue(ContextBuilder aPreviousStep, DebuggingData* aDebug) {
	if (aPreviousStep.question == 0) {
		return aPreviousStep;
	}
	if (aPreviousStep.question < 0) {
		aPreviousStep.question *= -1;
		switch (aPreviousStep.question) { // NOLINT
		case QuestionType_ListOfFolders:
			DoFolders(&aPreviousStep, aDebug);
			break;
		case QuestionType_ContentOfFile:
			DoFile(&aPreviousStep, aDebug);
			break;
		case QuestionType_ParseStringToPerson:
			ParsePerson(&aPreviousStep, aDebug);
			break;
		case QuestionType_ParseStringToRelations:
			ParseRelation(&aPreviousStep, aDebug);
			break;
		}
	}

	Internals* internals = (Internals*)aPreviousStep.data;

	switch (aPreviousStep.question) { // NOLINT
	case QuestionType_ParseStringToRelations:
		internals->context->relationCount = aPreviousStep.arrayCount;
		internals->context->relations = aPreviousStep.aRelations;

		aPreviousStep.question = QuestionType_ListOfFolders;
		aPreviousStep.qFoldername = internals->rootPath;
		break;
	case QuestionType_ListOfFolders:
		internals->foldernameCount = aPreviousStep.arrayCount;
		internals->foldernames = aPreviousStep.aStrings;
		internals->currentPos = 0;
		aPreviousStep.aPerson = NULL;
		// fallthrough intended
	case QuestionType_ParseStringToPerson:
		if (internals->currentPos >= internals->foldernameCount) {
			aPreviousStep.question = QuestionType_ContextIsReady;
			aPreviousStep.context = CreateContext(internals);
			FreeInternals(internals);
			aPreviousStep.data = NULL;
			break;
		}
		if (aPreviousStep.aPerson != NULL) {
			void* new = realloc(
				(void*)internals->context->persons,
				(internals->context->personCount + 1) * sizeof(PersonExt));
			if (new == NULL) {
				// TODO: deal with failed realloc
				break;
			}

			internals->context->persons = (PersonExt*)new;
			internals->context->persons[internals->context->personCount] = *aPreviousStep.aPerson;
			internals->context->personCount++;
		}

		size_t currentFoldernameLenght = strlen(internals->foldernames[internals->currentPos]);
		size_t newLength =
			internals->rootPathStrlen + currentFoldernameLenght + sizeof("data.json");
		char* path = calloc(newLength, sizeof(char));
		StoreString(path, internals);

		aPreviousStep.question = QuestionType_ContentOfFile;
		strcpy(path, internals->rootPath);
		strcpy(path + internals->rootPathStrlen, internals->foldernames[internals->currentPos]);
		strcpy(path + internals->rootPathStrlen + currentFoldernameLenght, "data.json");
		aPreviousStep.qFilename = path;
		break;
	case QuestionType_ContentOfFile:
		if (internals->isRelations > 0) {
			aPreviousStep.question = QuestionType_ParseStringToRelations;
		} else {
			aPreviousStep.question = QuestionType_ParseStringToPerson;
		}
		aPreviousStep.qString = aPreviousStep.aString;
		break;
	}

	return aPreviousStep;
}