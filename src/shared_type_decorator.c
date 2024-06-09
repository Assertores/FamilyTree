#include "internal_types.h"

#include <stdlib.h>

typedef struct {
	IDataProvider interface;
	size_t myCount;
	IDataProvider* myOriginal;
} SharedDataProvider;

typedef struct {
	IRelationals interface;
	size_t myCount;
	IRelationals* myOriginal;
} SharedRelationals;

typedef struct {
	IPersonals interface;
	size_t myCount;
	IPersonals* myOriginal;
} SharedPersonals;

typedef struct {
	IPlatform interface;
	size_t myCount;
	IPlatform* myOriginal;
} SharedPlatform;

IDataProvider*
SharedDataProvider_Copy(IDataProvider* aThis) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	self->myCount++;
	return aThis;
}

size_t
SharedDataProvider_GetAllIdsCount(IDataProvider* aThis) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	return self->myOriginal->GetAllIdsCount(self->myOriginal);
}

void
SharedDataProvider_GetAllIds(IDataProvider* aThis, PersonId* aOutId) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	self->myOriginal->GetAllIds(self->myOriginal, aOutId);
}

void
SharedDataProvider_GetPerson(IDataProvider* aThis, PersonId aId, Person* aOutPerson) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	self->myOriginal->GetPerson(self->myOriginal, aId, aOutPerson);
}

void
SharedDataProvider_PlayPerson(IDataProvider* aThis, PersonId aId) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	self->myOriginal->PlayPerson(self->myOriginal, aId);
}

void
SharedDataProvider_ShowImages(IDataProvider* aThis, PersonId aId) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	self->myOriginal->ShowImages(self->myOriginal, aId);
}

size_t
SharedDataProvider_GetAllRelationsOfIdCount(IDataProvider* aThis, PersonId aId) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	return self->myOriginal->GetAllRelationsOfIdCount(self->myOriginal, aId);
}

void
SharedDataProvider_GetAllRelationsOfId(IDataProvider* aThis, PersonId aId, Relation* aOutRelation) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	self->myOriginal->GetAllRelationsOfId(self->myOriginal, aId, aOutRelation);
}

RelationType
SharedDataProvider_GetRelationType(IDataProvider* aThis, Relation aRelation) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	return self->myOriginal->GetRelationType(self->myOriginal, aRelation);
}

void
SharedDataProvider_Free(IDataProvider* aThis) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	self->myCount--;
	if (self->myCount > 0) {
		return;
	}
	self->myOriginal->Free(self->myOriginal);
	free(self);
}

IDataProvider*
CreateSharedDataProviderDecorator(IDataProvider* aElement) {
	SharedDataProvider* result = calloc(1, sizeof(SharedDataProvider));

	result->interface.Copy = SharedDataProvider_Copy;
	result->interface.GetAllIdsCount = SharedDataProvider_GetAllIdsCount;
	result->interface.GetAllIds = SharedDataProvider_GetAllIds;
	result->interface.GetPerson = SharedDataProvider_GetPerson;
	result->interface.PlayPerson = SharedDataProvider_PlayPerson;
	result->interface.ShowImages = SharedDataProvider_ShowImages;
	result->interface.GetAllRelationsOfIdCount = SharedDataProvider_GetAllRelationsOfIdCount;
	result->interface.GetAllRelationsOfId = SharedDataProvider_GetAllRelationsOfId;
	result->interface.GetRelationType = SharedDataProvider_GetRelationType;
	result->interface.Free = SharedDataProvider_Free;

	result->myCount = 1;
	result->myOriginal = aElement;

	return &result->interface;
}

IRelationals*
SharedRelationals_Copy(IRelationals* aThis) {
	SharedRelationals* self = (SharedRelationals*)aThis;
	self->myCount++;
	return aThis;
}

size_t
SharedRelationals_GetAllIdsCount(IRelationals* aThis) {
	SharedRelationals* self = (SharedRelationals*)aThis;
	return self->myOriginal->GetAllIdsCount(self->myOriginal);
}

void
SharedRelationals_GetAllIds(IRelationals* aThis, PersonId* aOutId) {
	SharedRelationals* self = (SharedRelationals*)aThis;
	self->myOriginal->GetAllIds(self->myOriginal, aOutId);
}

size_t
SharedRelationals_GetAllRelationsOfIdCount(IRelationals* aThis, PersonId aId) {
	SharedRelationals* self = (SharedRelationals*)aThis;
	return self->myOriginal->GetAllRelationsOfIdCount(self->myOriginal, aId);
}

void
SharedRelationals_GetAllRelationsOfId(IRelationals* aThis, PersonId aId, Relation* aOutRelation) {
	SharedRelationals* self = (SharedRelationals*)aThis;
	self->myOriginal->GetAllRelationsOfId(self->myOriginal, aId, aOutRelation);
}

RelationType
SharedRelationals_GetRelationType(IRelationals* aThis, Relation aRelation) {
	SharedRelationals* self = (SharedRelationals*)aThis;
	return self->myOriginal->GetRelationType(self->myOriginal, aRelation);
}

void
SharedRelationals_Free(IRelationals* aThis) {
	SharedRelationals* self = (SharedRelationals*)aThis;
	self->myCount--;
	if (self->myCount > 0) {
		return;
	}
	self->myOriginal->Free(self->myOriginal);
	free(self);
}

IRelationals*
CreateSharedRelationalsDecorator(IRelationals* aElement) {
	SharedRelationals* result = calloc(1, sizeof(SharedRelationals));

	result->interface.Copy = SharedRelationals_Copy;
	result->interface.GetAllIdsCount = SharedRelationals_GetAllIdsCount;
	result->interface.GetAllIds = SharedRelationals_GetAllIds;
	result->interface.GetAllRelationsOfIdCount = SharedRelationals_GetAllRelationsOfIdCount;
	result->interface.GetAllRelationsOfId = SharedRelationals_GetAllRelationsOfId;
	result->interface.GetRelationType = SharedRelationals_GetRelationType;
	result->interface.Free = SharedRelationals_Free;

	result->myCount = 1;
	result->myOriginal = aElement;

	return &result->interface;
}

IPersonals*
SharedPersonals_Copy(IPersonals* aThis) {
	SharedPersonals* self = (SharedPersonals*)aThis;
	self->myCount++;
	return aThis;
}
size_t
SharedPersonals_GetAllIdsCount(IPersonals* aThis) {
	SharedPersonals* self = (SharedPersonals*)aThis;
	return self->myOriginal->GetAllIdsCount(self->myOriginal);
}

void
SharedPersonals_GetAllIds(IPersonals* aThis, PersonId* aOutId) {
	SharedPersonals* self = (SharedPersonals*)aThis;
	self->myOriginal->GetAllIds(self->myOriginal, aOutId);
}

void
SharedPersonals_GetPerson(IPersonals* aThis, PersonId aId, Person* aOutPerson) {
	SharedPersonals* self = (SharedPersonals*)aThis;
	self->myOriginal->GetPerson(self->myOriginal, aId, aOutPerson);
}

void
SharedPersonals_PlayPerson(IPersonals* aThis, PersonId aId) {
	SharedPersonals* self = (SharedPersonals*)aThis;
	self->myOriginal->PlayPerson(self->myOriginal, aId);
}

void
SharedPersonals_ShowImages(IPersonals* aThis, PersonId aId) {
	SharedPersonals* self = (SharedPersonals*)aThis;
	self->myOriginal->ShowImages(self->myOriginal, aId);
}

void
SharedPersonals_Free(IPersonals* aThis) {
	SharedPersonals* self = (SharedPersonals*)aThis;
	self->myCount--;
	if (self->myCount > 0) {
		return;
	}
	self->myOriginal->Free(self->myOriginal);
	free(self);
}

IPersonals*
CreateSharedPersonalsDecorator(IPersonals* aElement) {
	SharedPersonals* result = calloc(1, sizeof(SharedPersonals));

	result->interface.Copy = SharedPersonals_Copy;
	result->interface.GetAllIdsCount = SharedPersonals_GetAllIdsCount;
	result->interface.GetAllIds = SharedPersonals_GetAllIds;
	result->interface.GetPerson = SharedPersonals_GetPerson;
	result->interface.PlayPerson = SharedPersonals_PlayPerson;
	result->interface.ShowImages = SharedPersonals_ShowImages;
	result->interface.Free = SharedPersonals_Free;

	result->myCount = 1;
	result->myOriginal = aElement;

	return &result->interface;
}

IPlatform*
SharedPlatform_Copy(IPlatform* aThis) {
	SharedPlatform* self = (SharedPlatform*)aThis;
	self->myCount++;
	return aThis;
}

char*
SharedPlatform_GetFolders(IPlatform* aThis, const char* aPath) {
	SharedPlatform* self = (SharedPlatform*)aThis;
	return self->myOriginal->GetFolders(self->myOriginal, aPath);
}

char*
SharedPlatform_ReadFile(IPlatform* aThis, const char* aPath) {
	SharedPlatform* self = (SharedPlatform*)aThis;
	return self->myOriginal->ReadFile(self->myOriginal, aPath);
}

void
SharedPlatform_FreeString(IPlatform* aThis, char* aString) {
	SharedPlatform* self = (SharedPlatform*)aThis;
	self->myOriginal->FreeString(self->myOriginal, aString);
}

void
SharedPlatform_OpenAudio(IPlatform* aThis, const char* aPath) {
	SharedPlatform* self = (SharedPlatform*)aThis;
	self->myOriginal->OpenAudio(self->myOriginal, aPath);
}

void
SharedPlatform_OpenImage(IPlatform* aThis, const char* aPath) {
	SharedPlatform* self = (SharedPlatform*)aThis;
	self->myOriginal->OpenImage(self->myOriginal, aPath);
}

void
SharedPlatform_Free(IPlatform* aThis) {
	SharedPlatform* self = (SharedPlatform*)aThis;
	self->myCount--;
	if (self->myCount > 0) {
		return;
	}
	self->myOriginal->Free(self->myOriginal);
	free(self);
}

IPlatform*
CreateSharedPlatformDecorator(IPlatform* aElement) {
	SharedPlatform* result = calloc(1, sizeof(SharedPlatform));

	result->interface.Copy = SharedPlatform_Copy;
	result->interface.GetFolders = SharedPlatform_GetFolders;
	result->interface.ReadFile = SharedPlatform_ReadFile;
	result->interface.FreeString = SharedPlatform_FreeString;
	result->interface.OpenAudio = SharedPlatform_OpenAudio;
	result->interface.OpenImage = SharedPlatform_OpenImage;
	result->interface.Free = SharedPlatform_Free;

	result->myCount = 1;
	result->myOriginal = aElement;

	return &result->interface;
}
