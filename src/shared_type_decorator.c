#include "internal_types.h"
#include "patch.h"

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
SharedDataProvider_Copy(IDataProvider* aThis, ITrace* aTrace) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	aTrace->AddEvent(aTrace, "Intersept Copy and refcount instead");
	self->myCount++;
	return aThis;
}

size_t
SharedDataProvider_GetAllIdsCount(IDataProvider* aThis, ITrace* aTrace) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	return self->myOriginal->GetAllIdsCount(self->myOriginal,aTrace);
}

void
SharedDataProvider_GetAllIds(IDataProvider* aThis, PersonId* aOutId, ITrace* aTrace) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	self->myOriginal->GetAllIds(self->myOriginal, aOutId,aTrace);
}

Person
SharedDataProvider_GetPerson(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	return self->myOriginal->GetPerson(self->myOriginal, aId, aTrace);
}

void
SharedDataProvider_PlayPerson(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	self->myOriginal->PlayPerson(self->myOriginal, aId, aTrace);
}

void
SharedDataProvider_ShowImages(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	self->myOriginal->ShowImages(self->myOriginal, aId, aTrace);
}

size_t
SharedDataProvider_GetAllRelationsOfIdCount(IDataProvider* aThis, PersonId aId, ITrace* aTrace) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	return self->myOriginal->GetAllRelationsOfIdCount(self->myOriginal, aId, aTrace);
}

void
SharedDataProvider_GetAllRelationsOfId(
	IDataProvider* aThis, PersonId aId, Relation* aOutRelation, ITrace* aTrace) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	self->myOriginal->GetAllRelationsOfId(self->myOriginal, aId, aOutRelation, aTrace);
}

RelationType
SharedDataProvider_GetRelationType(IDataProvider* aThis, Relation aRelation, ITrace* aTrace) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	return self->myOriginal->GetRelationType(self->myOriginal, aRelation, aTrace);
}

void
SharedDataProvider_Free(IDataProvider* aThis, ITrace* aTrace) {
	SharedDataProvider* self = (SharedDataProvider*)aThis;
	self->myCount--;
	if (self->myCount > 0) {
		aTrace->AddEvent(aTrace, "Intersept Free because other references still exist");
		return;
	}
	self->myOriginal->Free(self->myOriginal, aTrace);
	free(self);
}

IDataProvider*
CreateSharedDataProviderDecorator(IDataProvider* aElement, ITrace* aTrace) {
	if (aElement == NULL) {
		return NULL;
	}
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

	aTrace->AddEvent(aTrace, "start refcount for IDataProvider");
	return &result->interface;
}

IRelationals*
SharedRelationals_Copy(IRelationals* aThis, ITrace* aTrace) {
	SharedRelationals* self = (SharedRelationals*)aThis;
	aTrace->AddEvent(aTrace, "Intersept Copy and refcount instead");
	self->myCount++;
	return aThis;
}

size_t
SharedRelationals_GetAllIdsCount(IRelationals* aThis, ITrace* aTrace) {
	SharedRelationals* self = (SharedRelationals*)aThis;
	return self->myOriginal->GetAllIdsCount(self->myOriginal, aTrace);
}

void
SharedRelationals_GetAllIds(IRelationals* aThis, PersonId* aOutId, ITrace* aTrace) {
	SharedRelationals* self = (SharedRelationals*)aThis;
	self->myOriginal->GetAllIds(self->myOriginal, aOutId, aTrace);
}

size_t
SharedRelationals_GetAllRelationsOfIdCount(IRelationals* aThis, PersonId aId, ITrace* aTrace) {
	SharedRelationals* self = (SharedRelationals*)aThis;
	return self->myOriginal->GetAllRelationsOfIdCount(self->myOriginal, aId, aTrace);
}

void
SharedRelationals_GetAllRelationsOfId(
	IRelationals* aThis, PersonId aId, Relation* aOutRelation, ITrace* aTrace) {
	SharedRelationals* self = (SharedRelationals*)aThis;
	self->myOriginal->GetAllRelationsOfId(self->myOriginal, aId, aOutRelation, aTrace);
}

RelationType
SharedRelationals_GetRelationType(IRelationals* aThis, Relation aRelation, ITrace* aTrace) {
	SharedRelationals* self = (SharedRelationals*)aThis;
	return self->myOriginal->GetRelationType(self->myOriginal, aRelation, aTrace);
}

void
SharedRelationals_Free(IRelationals* aThis, ITrace* aTrace) {
	SharedRelationals* self = (SharedRelationals*)aThis;
	self->myCount--;
	if (self->myCount > 0) {
		aTrace->AddEvent(aTrace, "Intersept Free because other references still exist");
		return;
	}
	self->myOriginal->Free(self->myOriginal, aTrace);
	free(self);
}

IRelationals*
CreateSharedRelationalsDecorator(IRelationals* aElement, ITrace* aTrace) {
	if (aElement == NULL) {
		return NULL;
	}
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

	aTrace->AddEvent(aTrace, "start refcount for IRelationals");
	return &result->interface;
}

IPersonals*
SharedPersonals_Copy(IPersonals* aThis, ITrace* aTrace) {
	SharedPersonals* self = (SharedPersonals*)aThis;
	aTrace->AddEvent(aTrace, "Intersept Copy and refcount instead");
	self->myCount++;
	return aThis;
}
size_t
SharedPersonals_GetAllIdsCount(IPersonals* aThis, ITrace* aTrace) {
	SharedPersonals* self = (SharedPersonals*)aThis;
	return self->myOriginal->GetAllIdsCount(self->myOriginal, aTrace);
}

void
SharedPersonals_GetAllIds(IPersonals* aThis, PersonId* aOutId, ITrace* aTrace) {
	SharedPersonals* self = (SharedPersonals*)aThis;
	self->myOriginal->GetAllIds(self->myOriginal, aOutId, aTrace);
}

Person
SharedPersonals_GetPerson(IPersonals* aThis, PersonId aId, ITrace* aTrace) {
	SharedPersonals* self = (SharedPersonals*)aThis;
	return self->myOriginal->GetPerson(self->myOriginal, aId, aTrace);
}

void
SharedPersonals_PlayPerson(IPersonals* aThis, PersonId aId, ITrace* aTrace) {
	SharedPersonals* self = (SharedPersonals*)aThis;
	self->myOriginal->PlayPerson(self->myOriginal, aId, aTrace);
}

void
SharedPersonals_ShowImages(IPersonals* aThis, PersonId aId, ITrace* aTrace) {
	SharedPersonals* self = (SharedPersonals*)aThis;
	self->myOriginal->ShowImages(self->myOriginal, aId, aTrace);
}

void
SharedPersonals_Free(IPersonals* aThis, ITrace* aTrace) {
	SharedPersonals* self = (SharedPersonals*)aThis;
	self->myCount--;
	if (self->myCount > 0) {
		aTrace->AddEvent(aTrace, "Intersept Free because other references still exist");
		return;
	}
	self->myOriginal->Free(self->myOriginal, aTrace);
	free(self);
}

IPersonals*
CreateSharedPersonalsDecorator(IPersonals* aElement, ITrace* aTrace) {
	if (aElement == NULL) {
		return NULL;
	}
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

	aTrace->AddEvent(aTrace, "start refcount for IPersonals");
	return &result->interface;
}

IPlatform*
SharedPlatform_Copy(IPlatform* aThis, ITrace* aTrace) {
	SharedPlatform* self = (SharedPlatform*)aThis;
	aTrace->AddEvent(aTrace, "Intersept Copy and refcount instead");
	self->myCount++;
	return aThis;
}

char*
SharedPlatform_GetFolders(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	SharedPlatform* self = (SharedPlatform*)aThis;
	return self->myOriginal->GetFolders(self->myOriginal, aPath, aTrace);
}

char*
SharedPlatform_ReadFile(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	SharedPlatform* self = (SharedPlatform*)aThis;
	return self->myOriginal->ReadFile(self->myOriginal, aPath, aTrace);
}

void
SharedPlatform_FreeString(IPlatform* aThis, char* aString, ITrace* aTrace) {
	SharedPlatform* self = (SharedPlatform*)aThis;
	self->myOriginal->FreeString(self->myOriginal, aString, aTrace);
}

void
SharedPlatform_OpenAudio(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	SharedPlatform* self = (SharedPlatform*)aThis;
	self->myOriginal->OpenAudio(self->myOriginal, aPath, aTrace);
}

void
SharedPlatform_OpenImage(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	SharedPlatform* self = (SharedPlatform*)aThis;
	self->myOriginal->OpenImage(self->myOriginal, aPath, aTrace);
}

void
SharedPlatform_Free(IPlatform* aThis, ITrace* aTrace) {
	SharedPlatform* self = (SharedPlatform*)aThis;
	self->myCount--;
	if (self->myCount > 0) {
		aTrace->AddEvent(aTrace, "Intersept Free because other references still exist");
		return;
	}
	self->myOriginal->Free(self->myOriginal, aTrace);
	free(self);
}

IPlatform*
CreateSharedPlatformDecorator(IPlatform* aElement, ITrace* aTrace) {
	if (aElement == NULL) {
		return NULL;
	}
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

	aTrace->AddEvent(aTrace, "start refcount for IPlatform");
	return &result->interface;
}
