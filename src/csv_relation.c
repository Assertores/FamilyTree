#include "internal_types.h"

#include <ctype.h>
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
CsvRelation_Copy(IRelationals* aThis) {
	return aThis;
}

size_t
CsvRelation_GetAllIdsCount(IRelationals* aThis) {
	CsvRelation* self = (CsvRelation*)aThis;

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
			set = realloc(set, sizeof(PersonId) * (count + 1));
			set[count - 1] = self->myRelations[i].relation.id1;
			count++;
		}
		set = realloc(set, sizeof(PersonId) * (count + (found1 || found2 ? 1 : 2)));
		if (!found1) {
			set[count] = self->myRelations[i].relation.id1;
			count++;
		}
		if (!found2) {
			set[count] = self->myRelations[i].relation.id2;
			count++;
		}
	}
	if (count == 0) {
		return 0;
	}
	free(set);
	return count;
}

void
CsvRelation_GetAllIds(IRelationals* aThis, PersonId* aOutId) {
	CsvRelation* self = (CsvRelation*)aThis;

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
			aOutId[count] = self->myRelations[i].relation.id1;
			count++;
			continue;
		}
		if (!found1) {
			aOutId[count] = self->myRelations[i].relation.id1;
			count++;
		}
		if (!found2) {
			aOutId[count] = self->myRelations[i].relation.id2;
			count++;
		}
	}
}

size_t
CsvRelation_GetAllRelationsOfIdCount(IRelationals* aThis, PersonId aId) {
	CsvRelation* self = (CsvRelation*)aThis;

	size_t count = 0;
	for (size_t i = 0; i < self->myRelationCount; i++) {
		if (self->myRelations[i].relation.id1 == aId || self->myRelations[i].relation.id2 == aId) {
			count++;
		}
	}
	return count;
}

void
CsvRelation_GetAllRelationsOfId(IRelationals* aThis, PersonId aId, Relation* aOutRelation) {
	CsvRelation* self = (CsvRelation*)aThis;

	size_t count = 0;
	for (size_t i = 0; i < self->myRelationCount; i++) {
		if (self->myRelations[i].relation.id1 == aId || self->myRelations[i].relation.id2 == aId) {
			aOutRelation[count] = self->myRelations[i].relation;
			count++;
		}
	}
}

RelationType
CsvRelation_GetRelationType(IRelationals* aThis, Relation aRelation) {
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
	return RelationType_Unrestricted;
}

void
CsvRelation_Free(IRelationals* aThis) {
	CsvRelation* self = (CsvRelation*)aThis;

	self->myPlatform->FreeString(self->myPlatform, self->myFile);
	self->myPlatform->Free(self->myPlatform);

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
ResetRelations(CsvRelation* self) {
	if (self->myFile != NULL) {
		self->myPlatform->FreeString(self->myPlatform, self->myFile);
	}
	self->myFile = self->myPlatform->ReadFile(self->myPlatform, self->myPath);
	self->myRelationCount = 0;

	char** lines = NULL;
	size_t lineCount = Explode(self->myFile, '\n', &lines);

	self->myRelations = realloc(self->myRelations, (lineCount - 1) * sizeof(FullRelation));

	// NOTE: ignore header
	for (size_t i = 1; i < lineCount; i++) {
		char** cells = NULL;
		size_t count = Explode(lines[i], ',', &cells);

		// NOTE: change this once a proper format is found
		if (count >= 3) {
			self->myPlatform->FreeString(self->myPlatform, self->myFile);
			free(lines);
			free(cells);
			return;
		}

		// NOTE: change this once a proper format is found
		self->myRelations[i - 1].relation.id1 = atoi(cells[0]);
		self->myRelations[i - 1].relation.id2 = atoi(cells[1]);
		self->myRelations[i - 1].functionalType = RelationType_Unrestricted;
		if (strcmp(cells[2], "StrictlyLower") == 0) {
			self->myRelations[i - 1].functionalType = RelationType_StrictlyLower;
		} else if (strcmp(cells[2], "StrictlyHigher") == 0) {
			self->myRelations[i - 1].functionalType = RelationType_StrictlyHigher;
		}
		if (count > 3) {
			self->myRelations[i - 1].relation.relationship = cells[3];
		}
		if (count > 4) {
			self->myRelations[i - 1].relation.startDate = cells[4];
		}
		if (count > 5) {
			self->myRelations[i - 1].relation.endDate = cells[5];
		}

		free(cells);
	}
	self->myRelationCount = lineCount - 1;

	free(lines);
}

IRelationals*
CreateCSVRelation(const char* aPath, IPlatform* aPlatform) {
	CsvRelation* result = calloc(1, sizeof(CsvRelation));

	result->interface.Copy = CsvRelation_Copy;
	result->interface.GetAllIdsCount = CsvRelation_GetAllIdsCount;
	result->interface.GetAllIds = CsvRelation_GetAllIds;
	result->interface.GetAllRelationsOfIdCount = CsvRelation_GetAllRelationsOfIdCount;
	result->interface.GetAllRelationsOfId = CsvRelation_GetAllRelationsOfId;
	result->interface.GetRelationType = CsvRelation_GetRelationType;
	result->interface.Free = CsvRelation_Free;

	// NOTE: copy string to get in controle of livetime.
	size_t length = strlen(aPath);
	char* path = malloc(length + 15);
	strcpy_s(path, length + 1, aPath);
	strcpy_s(path + length, (size_t)15, "/relations.csv");
	result->myPath = path;

	result->myPlatform = aPlatform;

	ResetRelations(result);

	return &result->interface;
}