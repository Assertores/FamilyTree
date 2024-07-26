#include "internal_types.h"
#include "patch.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct FullRelation {
	Relation relation;
	RelationType functionalType;
} FullRelation;

typedef struct {
	IRelationals interface;

	char* myFile;
	char* myPath;
	IPlatform* myPlatform;

	size_t myRelationCount;
	FullRelation* myRelations;
} CsvRelation;

IRelationals*
CsvRelation_Copy(IRelationals* aThis, ITrace* aTrace) {
	return aThis;
}

size_t
CsvRelation_GetAllIdsCount(IRelationals* aThis, ITrace* aTrace) {
	CsvRelation* self = (CsvRelation*)aThis;
	ITrace* trace = aTrace->CreateSubTrace(aTrace, "Count Unique Ids from CSV relations");
	char string[126];

	size_t count = 0;
	PersonId* set = NULL;
	for (size_t i = 0; i < self->myRelationCount; i++) {
		int found1 = 0;
		int found2 = 0;
		for (size_t j = 0; j < count; j++) {
			if (set[j] == self->myRelations[i].relation.id1) {
				found1 = 1;
			}
			if (set[j] == self->myRelations[i].relation.id2) {
				found2 = 1;
			}
			if (found1 && found2) {
				break;
			}
		}
		if (found1 && found2) {
			continue;
		}
		if (self->myRelations[i].relation.id1 == self->myRelations[i].relation.id2) {
			sprintf(string, "Found new id: %zu", self->myRelations[i].relation.id1);
			trace->AddEvent(trace, string);
			set = realloc(set, sizeof(PersonId) * (count + 1));
			set[count - 1] = self->myRelations[i].relation.id1;
			count++;
		}
		set = realloc(set, sizeof(PersonId) * (count + (found1 || found2 ? 1 : 2)));
		if (!found1) {
			sprintf(string, "Found new id: %zu", self->myRelations[i].relation.id1);
			trace->AddEvent(trace, string);
			set[count] = self->myRelations[i].relation.id1;
			count++;
		}
		if (!found2) {
			sprintf(string, "Found new id: %zu", self->myRelations[i].relation.id2);
			trace->AddEvent(trace, string);
			set[count] = self->myRelations[i].relation.id2;
			count++;
		}
	}
	if (count == 0) {
		trace->Free(trace);
		return 0;
	}

	trace->AddEvent(trace, "Free Memory of Helping structure");
	trace->Free(trace);
	free(set);
	return count;
}

void
CsvRelation_GetAllIds(IRelationals* aThis, PersonId* aOutId, ITrace* aTrace) {
	CsvRelation* self = (CsvRelation*)aThis;
	ITrace* trace = aTrace->CreateSubTrace(aTrace, "Retrieve Unique Ids from CSV relations");
	char string[126];

	size_t count = 0;
	for (size_t i = 0; i < self->myRelationCount; i++) {
		int found1 = 0;
		int found2 = 0;
		for (size_t j = 0; j < count; j++) {
			if (aOutId[j] == self->myRelations[i].relation.id1) {
				found1 = 1;
			}
			if (aOutId[j] == self->myRelations[i].relation.id2) {
				found2 = 1;
			}
			if (found1 && found2) {
				break;
			}
		}
		if (found1 && found2) {
			continue;
		}
		if (self->myRelations[i].relation.id1 == self->myRelations[i].relation.id2) {
			sprintf(string, "Found new id: %zu", self->myRelations[i].relation.id1);
			trace->AddEvent(trace, string);
			aOutId[count] = self->myRelations[i].relation.id1;
			count++;
			continue;
		}
		if (!found1) {
			sprintf(string, "Found new id: %zu", self->myRelations[i].relation.id1);
			trace->AddEvent(trace, string);
			aOutId[count] = self->myRelations[i].relation.id1;
			count++;
		}
		if (!found2) {
			sprintf(string, "Found new id: %zu", self->myRelations[i].relation.id2);
			trace->AddEvent(trace, string);
			aOutId[count] = self->myRelations[i].relation.id2;
			count++;
		}
	}
	trace->Free(trace);
}

size_t
CsvRelation_GetAllRelationsOfIdCount(IRelationals* aThis, PersonId aId, ITrace* aTrace) {
	CsvRelation* self = (CsvRelation*)aThis;
	char string[126];
	sprintf(string, "Count relations for person: %zu", aId);
	ITrace* trace = aTrace->CreateSubTrace(aTrace, string);

	size_t count = 0;
	for (size_t i = 0; i < self->myRelationCount; i++) {
		if (self->myRelations[i].relation.id1 == aId || self->myRelations[i].relation.id2 == aId) {
			count++;
		}
	}
	sprintf(string, "Found %zu relations", count);
	trace->AddEvent(trace, string);
	trace->Free(trace);
	return count;
}

void
CsvRelation_GetAllRelationsOfId(
	IRelationals* aThis, PersonId aId, Relation* aOutRelation, ITrace* aTrace) {
	CsvRelation* self = (CsvRelation*)aThis;
	char string[126];
	sprintf(string, "Retrieve relations for person: %zu", aId);
	ITrace* trace = aTrace->CreateSubTrace(aTrace, string);

	size_t count = 0;
	for (size_t i = 0; i < self->myRelationCount; i++) {
		if (self->myRelations[i].relation.id1 == aId || self->myRelations[i].relation.id2 == aId) {
			trace->AddEvent(trace, "Found another one");
			aOutRelation[count] = self->myRelations[i].relation;
			count++;
		}
	}
	trace->Free(trace);
}

RelationType
CsvRelation_GetRelationType(IRelationals* aThis, Relation aRelation, ITrace* aTrace) {
	CsvRelation* self = (CsvRelation*)aThis;

	for (size_t i = 0; i < self->myRelationCount; i++) {
		if (self->myRelations[i].relation.id1 != aRelation.id1) {
			continue;
		}
		if (self->myRelations[i].relation.id2 != aRelation.id2) {
			continue;
		}
		return self->myRelations[i].functionalType;
	}
	aTrace->AddEvent(aTrace, "Provided relation is unrecogniced");
	return RelationType_Unrestricted;
}

void
CsvRelation_Free(IRelationals* aThis, ITrace* aTrace) {
	CsvRelation* self = (CsvRelation*)aThis;

	self->myPlatform->FreeString(self->myPlatform, self->myFile, aTrace);
	self->myPlatform->Free(self->myPlatform, aTrace);

	free(self->myPath);
	free(self->myRelations);
	free(self);
}

size_t
Explode(char* aSrc, char aDelimiter, char*** aOutArray) {
	size_t length = strlen(aSrc);

	size_t count = 0;
	for (size_t i = 0; i < length; i++) {
		if (aSrc[i] == aDelimiter) {
			count++;
		}
	}
	if (count == 0) {
		*aOutArray = malloc(sizeof(char*));
		*aOutArray[0] = aSrc;
		return 1;
	}
	count++;

	*aOutArray = malloc(count * sizeof(char*));
	size_t index = 0;
	*aOutArray[0] = aSrc;
	for (size_t i = 1; i < length; i++) {
		if (aSrc[i] != aDelimiter) {
			continue;
		}

		aSrc[i] = '\0';
		index++;
		(*aOutArray)[index] = aSrc + i + 1;
	}
	return count;
}

void
ResetRelations(CsvRelation* self, ITrace* aTrace) {
	ITrace* trace = aTrace->CreateSubTrace(aTrace, "Reset Relations");

	if (self->myFile != NULL) {
		trace->AddEvent(trace, "Freeing old file");
		self->myPlatform->FreeString(self->myPlatform, self->myFile, trace);
	}
	self->myFile = self->myPlatform->ReadFile(self->myPlatform, self->myPath, trace);
	self->myRelationCount = 0;

	trace->AddEvent(trace, "Split file into lines");
	char** lines = NULL;
	size_t lineCount = Explode(self->myFile, '\n', &lines);

	self->myRelations = realloc(self->myRelations, (lineCount - 1) * sizeof(FullRelation));

	// NOTE: ignore header
	for (size_t i = 1; i < lineCount; i++) {
		char** cells = NULL;
		size_t count = Explode(lines[i], ',', &cells);

		if (count < 3) {
			char string[2048];
			sprintf(
				string,
				"csv file %s has only %zu elements in line %zu, minimum elements is 3",
				self->myPath,
				count,
				i);
			trace->Fail(trace, string);
			trace->Free(trace);

			free(lines);
			free(cells);
			return;
		}

		self->myRelations[i - 1].relation.id1 = atoi(cells[0]);
		self->myRelations[i - 1].relation.id2 = atoi(cells[1]);
		self->myRelations[i - 1].functionalType = RelationType_Unrestricted;
		if (strcmp(cells[2], "StrictlyLower") == 0) {
			self->myRelations[i - 1].functionalType = RelationType_StrictlyLower;
		} else if (strcmp(cells[2], "StrictlyHigher") == 0) {
			self->myRelations[i - 1].functionalType = RelationType_StrictlyHigher;
		}
		self->myRelations[i - 1].relation.relationship = NULL;
		self->myRelations[i - 1].relation.startDate = NULL;
		self->myRelations[i - 1].relation.endDate = NULL;
		if (count > 3 && strlen(cells[3]) > 0) {
			self->myRelations[i - 1].relation.relationship = cells[3];
		}
		if (count > 4 && strlen(cells[4]) > 0) {
			self->myRelations[i - 1].relation.startDate = cells[4];
		}
		if (count > 5 && strlen(cells[5]) > 0) {
			self->myRelations[i - 1].relation.endDate = cells[5];
		}

		free(cells);
	}
	self->myRelationCount = lineCount - 1;

	char string[126];
	sprintf(string, "Tracking %zu relations", self->myRelationCount);
	trace->AddEvent(trace, string);

	free(lines);
	trace->Free(trace);
}

IRelationals*
CreateCSVRelation(const char* aPath, IPlatform* aPlatform, ITrace* aTrace) {
	ITrace* trace = aTrace->CreateSubTrace(aTrace, "Construct CSV Relation");
	trace->AddEvent(trace, "Allocate memory");
	CsvRelation* result = calloc(1, sizeof(CsvRelation));

	trace->AddEvent(trace, "Set Dispatch Table");
	result->interface.Copy = CsvRelation_Copy;
	result->interface.GetAllIdsCount = CsvRelation_GetAllIdsCount;
	result->interface.GetAllIds = CsvRelation_GetAllIds;
	result->interface.GetAllRelationsOfIdCount = CsvRelation_GetAllRelationsOfIdCount;
	result->interface.GetAllRelationsOfId = CsvRelation_GetAllRelationsOfId;
	result->interface.GetRelationType = CsvRelation_GetRelationType;
	result->interface.Free = CsvRelation_Free;

	trace->AddEvent(trace, "Copy string to get in controle of livetime");
	size_t length = strlen(aPath);
	char* path = malloc(length + 15);
	strcpy_s(path, length + 1, aPath);
	strcpy_s(path + length, (size_t)15, "/relations.csv");
	result->myPath = path;

	result->myPlatform = aPlatform;

	ResetRelations(result, trace);

	return &result->interface;
}