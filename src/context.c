#include "algorythms.h"
#include "family_tree/api.h"
#include "internal_types.h"
#include "patch.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define FreeTraceAndReturn    \
	if (IsNoOpTrace) {        \
		aTrace->Free(aTrace); \
	}                         \
	return

#define Subtrace(aName, aCode)                        \
	subtrace = aTrace->CreateSubTrace(aTrace, aName); \
	aCode;                                            \
	subtrace->Free(subtrace)

struct Context {
	ProviderComposit* myDataProvider;
	MetaData myMedaData;

	const char* myDefaultString;

	size_t myCreatedDataProvidersSize;
	IDataProvider** myCreatedDataProviders;
	size_t myCreatedRelationsSize;
	IRelationals** myCreatedRelations;
	size_t myCreatedPersonalsSize;
	IPersonals** myCreatedPersonals;
	size_t myCreatedPlatformSize;
	IPlatform** myCreatedPlatforms;
	Person* myCreatedPersons;
};

FT_API Context*
Create(ITrace* aTrace) {
	int IsNoOpTrace = 0;
	if (aTrace == NULL || aTrace->CreateSubTrace == NULL || aTrace->AddEvent == NULL
		|| aTrace->Succeed == NULL || aTrace->Fail == NULL || aTrace->Free == NULL) {
		IsNoOpTrace = 1;
		aTrace = CreateNoOpTrace();
	}

	Context* context = calloc(1, sizeof(Context));
	if (context == NULL) {
		aTrace->AddEvent(aTrace, "Context is NULL");
		FreeTraceAndReturn NULL;
	}
	context->myDataProvider = CreateProviderComposit();
	if (context->myDataProvider == NULL) {
		FreeTraceAndReturn NULL;
	}
	context->myMedaData.myPersonCount = 0;
	context->myMedaData.myPersons = NULL;
	context->myDefaultString = "N/A";

	FreeTraceAndReturn context;
}

FT_API void
Free(Context* aContext, ITrace* aTrace) {
	int IsNoOpTrace = 0;
	if (aTrace == NULL || aTrace->CreateSubTrace == NULL || aTrace->AddEvent == NULL
		|| aTrace->Succeed == NULL || aTrace->Fail == NULL || aTrace->Free == NULL) {
		IsNoOpTrace = 1;
		aTrace = CreateNoOpTrace();
	}
	if (aContext == NULL) {
		aTrace->AddEvent(aTrace, "Context is NULL");
		FreeTraceAndReturn;
	}

	IDataProvider* interface = ProviderComposit_Cast(aContext->myDataProvider);
	interface->Free(interface);

	for (size_t i = 0; i < aContext->myCreatedDataProvidersSize; i++) {
		aContext->myCreatedDataProviders[i]->Free(aContext->myCreatedDataProviders[i]);
	}
	free(aContext->myCreatedDataProviders);

	for (size_t i = 0; i < aContext->myCreatedRelationsSize; i++) {
		aContext->myCreatedRelations[i]->Free(aContext->myCreatedRelations[i]);
	}
	free(aContext->myCreatedRelations);

	for (size_t i = 0; i < aContext->myCreatedPersonalsSize; i++) {
		aContext->myCreatedPersonals[i]->Free(aContext->myCreatedPersonals[i]);
	}
	free(aContext->myCreatedPersonals);

	for (size_t i = 0; i < aContext->myCreatedPlatformSize; i++) {
		aContext->myCreatedPlatforms[i]->Free(aContext->myCreatedPlatforms[i]);
	}
	free(aContext->myCreatedPlatforms);

	free(aContext->myCreatedPersons);
	FreeMetaData(&aContext->myMedaData);

	free(aContext);

	FreeTraceAndReturn;
}

FT_API Context*
CreateWithCSVAndJSON(const char* aPath, ITrace* aTrace) {
	int IsNoOpTrace = 0;
	if (aTrace == NULL || aTrace->CreateSubTrace == NULL || aTrace->AddEvent == NULL
		|| aTrace->Succeed == NULL || aTrace->Fail == NULL || aTrace->Free == NULL) {
		IsNoOpTrace = 1;
		aTrace = CreateNoOpTrace();
	}

	ITrace* subtrace;
	Subtrace("Create", Context* context = Create(subtrace));
	if (context == NULL) {
		aTrace->AddEvent(aTrace, "Context is NULL");
		FreeTraceAndReturn context;
	}

	Subtrace(
		"CreateDefaultPlatform",
		IPlatform* platform = CreateDefaultPlatform(context, subtrace));
	Subtrace(
		"CreateCSVRelations",
		IRelationals* relations = CreateCSVRelations(context, aPath, platform, subtrace));
	Subtrace(
		"CreateJSONPersonals",
		IPersonals* personals = CreateJSONPersonals(context, aPath, platform, subtrace));
	Subtrace(
		"CreateDataProvider",
		IDataProvider* provider = CreateDataProvider(context, relations, personals, subtrace));
	Subtrace("AddDataProvider", AddDataProvider(context, provider, subtrace));

	FreeTraceAndReturn context;
}

FT_API Context*
CreateCSVAndJSONWithIO(const char* aPath, IPlatform* aPlatform, ITrace* aTrace) {
	int IsNoOpTrace = 0;
	if (aTrace == NULL || aTrace->CreateSubTrace == NULL || aTrace->AddEvent == NULL
		|| aTrace->Succeed == NULL || aTrace->Fail == NULL || aTrace->Free == NULL) {
		IsNoOpTrace = 1;
		aTrace = CreateNoOpTrace();
	}

	ITrace* subtrace;
	Subtrace("Create", Context* context = Create(subtrace));
	if (context == NULL) {
		aTrace->AddEvent(aTrace, "Context is NULL");
		FreeTraceAndReturn context;
	}

	Subtrace(
		"CreateCSVRelations",
		IRelationals* relations = CreateCSVRelations(context, aPath, aPlatform, subtrace));
	Subtrace(
		"CreateJSONPersonals",
		IPersonals* personals = CreateJSONPersonals(context, aPath, aPlatform, subtrace));
	Subtrace(
		"CreateDataProvider",
		IDataProvider* provider = CreateDataProvider(context, relations, personals, subtrace));
	Subtrace("AddDataProvider", AddDataProvider(context, provider, subtrace));

	FreeTraceAndReturn context;
}

FT_API void
AddDataProvider(Context* aContext, IDataProvider* aData, ITrace* aTrace) {
	int IsNoOpTrace = 0;
	if (aTrace == NULL || aTrace->CreateSubTrace == NULL || aTrace->AddEvent == NULL
		|| aTrace->Succeed == NULL || aTrace->Fail == NULL || aTrace->Free == NULL) {
		IsNoOpTrace = 1;
		aTrace = CreateNoOpTrace();
	}

	if (aContext == NULL) {
		aTrace->AddEvent(aTrace, "Context is NULL");
		FreeTraceAndReturn;
	}
	if (aData == NULL) {
		FreeTraceAndReturn;
	}
	if (aData->Copy == NULL) {
		FreeTraceAndReturn;
	}
	if (aData->GetAllIds == NULL) {
		FreeTraceAndReturn;
	}
	if (aData->GetAllIdsCount == NULL) {
		FreeTraceAndReturn;
	}
	if (aData->GetAllRelationsOfId == NULL) {
		FreeTraceAndReturn;
	}
	if (aData->GetAllRelationsOfIdCount == NULL) {
		FreeTraceAndReturn;
	}
	if (aData->GetPerson == NULL) {
		FreeTraceAndReturn;
	}
	if (aData->PlayPerson == NULL) {
		FreeTraceAndReturn;
	}

	ProviderComposit_AddDataProvider(aContext->myDataProvider, aData->Copy(aData));
	FreeMetaData(&aContext->myMedaData);
	aContext->myMedaData = CreateMetaData(ProviderComposit_Cast(aContext->myDataProvider));

	FreeTraceAndReturn;
}

FT_API IDataProvider*
CreateDataProvider(
	Context* aContext, IRelationals* aRelations, IPersonals* aPersonals, ITrace* aTrace) {
	int IsNoOpTrace = 0;
	if (aTrace == NULL || aTrace->CreateSubTrace == NULL || aTrace->AddEvent == NULL
		|| aTrace->Succeed == NULL || aTrace->Fail == NULL || aTrace->Free == NULL) {
		IsNoOpTrace = 1;
		aTrace = CreateNoOpTrace();
	}

	if (aContext == NULL) {
		aTrace->AddEvent(aTrace, "Context is NULL");
		FreeTraceAndReturn NULL;
	}
	if (aRelations->Copy == NULL) {
		FreeTraceAndReturn NULL;
	}
	if (aRelations->Free == NULL) {
		FreeTraceAndReturn NULL;
	}
	if (aRelations->GetAllIds == NULL) {
		FreeTraceAndReturn NULL;
	}
	if (aRelations->GetAllIdsCount == NULL) {
		FreeTraceAndReturn NULL;
	}
	if (aRelations->GetAllRelationsOfId == NULL) {
		FreeTraceAndReturn NULL;
	}
	if (aRelations->GetAllRelationsOfIdCount == NULL) {
		FreeTraceAndReturn NULL;
	}
	if (aPersonals->Copy == NULL) {
		FreeTraceAndReturn NULL;
	}
	if (aPersonals->Free == NULL) {
		FreeTraceAndReturn NULL;
	}
	if (aPersonals->GetAllIds == NULL) {
		FreeTraceAndReturn NULL;
	}
	if (aPersonals->GetAllIdsCount == NULL) {
		FreeTraceAndReturn NULL;
	}
	if (aPersonals->GetPerson == NULL) {
		FreeTraceAndReturn NULL;
	}
	if (aPersonals->PlayPerson == NULL) {
		FreeTraceAndReturn NULL;
	}
	if (aPersonals->ShowImages == NULL) {
		FreeTraceAndReturn NULL;
	}

	IDataProvider* data = CreateSharedDataProviderDecorator(
		CreateForwardingProvider(aRelations->Copy(aRelations), aPersonals->Copy(aPersonals)));
	if (data == NULL) {
		FreeTraceAndReturn NULL;
	}

	IDataProvider** newArray = realloc(
		aContext->myCreatedDataProviders,
		sizeof(IRelationals*) * (aContext->myCreatedDataProvidersSize + 1));
	if (newArray == NULL) {
		data->Free(data);
		free(data);
		FreeTraceAndReturn NULL;
	}

	newArray[aContext->myCreatedDataProvidersSize] = data;
	aContext->myCreatedDataProvidersSize++;
	aContext->myCreatedDataProviders = newArray;

	FreeTraceAndReturn data;
}

int
CheckIo(IPlatform* aPlatform) {
	if (aPlatform == NULL) {
		return 0;
	}
	if (aPlatform->Copy == NULL) {
		return 0;
	}
	if (aPlatform->GetFolders == NULL) {
		return 0;
	}
	if (aPlatform->ReadFile == NULL) {
		return 0;
	}
	if (aPlatform->FreeString == NULL) {
		return 0;
	}
	if (aPlatform->OpenAudio == NULL) {
		return 0;
	}
	if (aPlatform->OpenImage == NULL) {
		return 0;
	}
	if (aPlatform->Free == NULL) {
		return 0;
	}
	return 1;
}

FT_API IRelationals*
CreateCSVRelations(Context* aContext, const char* aPath, IPlatform* aPlatform, ITrace* aTrace) {
	int IsNoOpTrace = 0;
	if (aTrace == NULL || aTrace->CreateSubTrace == NULL || aTrace->AddEvent == NULL
		|| aTrace->Succeed == NULL || aTrace->Fail == NULL || aTrace->Free == NULL) {
		IsNoOpTrace = 1;
		aTrace = CreateNoOpTrace();
	}

	if (aContext == NULL) {
		aTrace->AddEvent(aTrace, "Context is NULL");
		FreeTraceAndReturn NULL;
	}
	if (!CheckIo(aPlatform)) {
		FreeTraceAndReturn NULL;
	}

	IRelationals* relations =
		CreateSharedRelationalsDecorator(CreateCSVRelation(aPath, aPlatform->Copy(aPlatform)));
	if (relations == NULL) {
		FreeTraceAndReturn NULL;
	}

	IRelationals** newArray = realloc(
		aContext->myCreatedRelations,
		sizeof(IRelationals*) * (aContext->myCreatedRelationsSize + 1));
	if (newArray == NULL) {
		relations->Free(relations);
		free(relations);
		FreeTraceAndReturn NULL;
	}

	newArray[aContext->myCreatedRelationsSize] = relations;
	aContext->myCreatedRelationsSize++;
	aContext->myCreatedRelations = newArray;
	FreeTraceAndReturn relations;
}

FT_API IPersonals*
CreateJSONPersonals(Context* aContext, const char* aPath, IPlatform* aPlatform, ITrace* aTrace) {
	int IsNoOpTrace = 0;
	if (aTrace == NULL || aTrace->CreateSubTrace == NULL || aTrace->AddEvent == NULL
		|| aTrace->Succeed == NULL || aTrace->Fail == NULL || aTrace->Free == NULL) {
		IsNoOpTrace = 1;
		aTrace = CreateNoOpTrace();
	}

	if (aContext == NULL) {
		aTrace->AddEvent(aTrace, "Context is NULL");
		FreeTraceAndReturn NULL;
	}
	if (!CheckIo(aPlatform)) {
		FreeTraceAndReturn NULL;
	}

	IPersonals* persons =
		CreateSharedPersonalsDecorator(CreateJSONPerson(aPath, aPlatform->Copy(aPlatform)));
	if (persons == NULL) {
		FreeTraceAndReturn NULL;
	}

	IPersonals** newArray = realloc(
		aContext->myCreatedPersonals,
		sizeof(IRelationals*) * (aContext->myCreatedPersonalsSize + 1));
	if (newArray == NULL) {
		persons->Free(persons);
		free(persons);
		FreeTraceAndReturn NULL;
	}

	newArray[aContext->myCreatedPersonalsSize] = persons;
	aContext->myCreatedPersonalsSize++;
	aContext->myCreatedPersonals = newArray;
	FreeTraceAndReturn persons;
}

FT_API IPlatform*
CreateDefaultPlatform(Context* aContext, ITrace* aTrace) {
	int IsNoOpTrace = 0;
	if (aTrace == NULL || aTrace->CreateSubTrace == NULL || aTrace->AddEvent == NULL
		|| aTrace->Succeed == NULL || aTrace->Fail == NULL || aTrace->Free == NULL) {
		IsNoOpTrace = 1;
		aTrace = CreateNoOpTrace();
	}

	if (aContext == NULL) {
		aTrace->AddEvent(aTrace, "Context is NULL");
		FreeTraceAndReturn NULL;
	}

	IPlatform* platform = CreateSharedPlatformDecorator(CreatePlatform());
	if (platform == NULL) {
		FreeTraceAndReturn NULL;
	}

	IPlatform** newArray = realloc(
		aContext->myCreatedPlatforms,
		sizeof(IRelationals*) * (aContext->myCreatedPlatformSize + 1));
	if (newArray == NULL) {
		platform->Free(platform);
		free(platform);
		FreeTraceAndReturn NULL;
	}

	newArray[aContext->myCreatedPlatformSize] = platform;
	aContext->myCreatedPlatformSize++;
	aContext->myCreatedPlatforms = newArray;
	FreeTraceAndReturn platform;
}

void
PopulateNullValues(Person* aPerson, Context* aContext) {
	if (aPerson->title == NULL) {
		aPerson->title = aContext->myDefaultString;
	}
	if (aPerson->firstNameCount == 0) {
		aPerson->firstNames = &aContext->myDefaultString;
	}
	if (aPerson->titleOfNobility == NULL) {
		aPerson->titleOfNobility = aContext->myDefaultString;
	}
	if (aPerson->lastNameCount == 0) {
		aPerson->lastNames = &aContext->myDefaultString;
	}
	if (aPerson->gender == NULL) {
		aPerson->gender = aContext->myDefaultString;
	}
	if (aPerson->dateOfBirth == NULL) {
		aPerson->dateOfBirth = aContext->myDefaultString;
	}
	if (aPerson->placeOfBirth == NULL) {
		aPerson->placeOfBirth = aContext->myDefaultString;
	}
	if (aPerson->dateOfDeath == NULL) {
		aPerson->dateOfDeath = aContext->myDefaultString;
	}
	if (aPerson->placeOfDeath == NULL) {
		aPerson->placeOfDeath = aContext->myDefaultString;
	}
	if (aPerson->remark == NULL) {
		aPerson->remark = aContext->myDefaultString;
	}
}

FT_API Person
GetPerson(Context* aContext, PersonId aId, ITrace* aTrace) {
	int IsNoOpTrace = 0;
	if (aTrace == NULL || aTrace->CreateSubTrace == NULL || aTrace->AddEvent == NULL
		|| aTrace->Succeed == NULL || aTrace->Fail == NULL || aTrace->Free == NULL) {
		IsNoOpTrace = 1;
		aTrace = CreateNoOpTrace();
	}

	Person p;
	p.id = 0;
	p.title = "";
	p.firstNameCount = 0;
	p.firstNames = &p.title;
	p.titleOfNobility = "";
	p.lastNameCount = 0;
	p.lastNames = &p.title;
	p.gender = "";
	p.dateOfBirth = "";
	p.placeOfBirth = "";
	p.dateOfDeath = "";
	p.placeOfDeath = "";
	p.remark = "";

	if (aContext == NULL) {
		aTrace->AddEvent(aTrace, "Context is NULL");
		FreeTraceAndReturn p;
	}

	IDataProvider* interface = ProviderComposit_Cast(aContext->myDataProvider);
	p = interface->GetPerson(interface, aId);
	PopulateNullValues(&p, aContext);
	FreeTraceAndReturn p;
}

FT_API void
PlayPerson(Context* aContext, PersonId aId, ITrace* aTrace) {
	int IsNoOpTrace = 0;
	if (aTrace == NULL || aTrace->CreateSubTrace == NULL || aTrace->AddEvent == NULL
		|| aTrace->Succeed == NULL || aTrace->Fail == NULL || aTrace->Free == NULL) {
		IsNoOpTrace = 1;
		aTrace = CreateNoOpTrace();
	}

	if (aContext == NULL) {
		aTrace->AddEvent(aTrace, "Context is NULL");
		FreeTraceAndReturn;
	}

	IDataProvider* interface = ProviderComposit_Cast(aContext->myDataProvider);
	interface->PlayPerson(interface, aId);
	FreeTraceAndReturn;
}

FT_API void
ShowImagesOfPerson(Context* aContext, PersonId aId, ITrace* aTrace) {
	int IsNoOpTrace = 0;
	if (aTrace == NULL || aTrace->CreateSubTrace == NULL || aTrace->AddEvent == NULL
		|| aTrace->Succeed == NULL || aTrace->Fail == NULL || aTrace->Free == NULL) {
		IsNoOpTrace = 1;
		aTrace = CreateNoOpTrace();
	}

	if (aContext == NULL) {
		aTrace->AddEvent(aTrace, "Context is NULL");
		FreeTraceAndReturn;
	}

	IDataProvider* interface = ProviderComposit_Cast(aContext->myDataProvider);
	interface->ShowImages(interface, aId);
	FreeTraceAndReturn;
}

FT_API Person*
GetPersonsMatchingPattern(
	Context* aContext,
	Person aPrototype,
	size_t aMinMatches,
	size_t* aOutPersonsCount,
	ITrace* aTrace) {
	int IsNoOpTrace = 0;
	if (aTrace == NULL || aTrace->CreateSubTrace == NULL || aTrace->AddEvent == NULL
		|| aTrace->Succeed == NULL || aTrace->Fail == NULL || aTrace->Free == NULL) {
		IsNoOpTrace = 1;
		aTrace = CreateNoOpTrace();
	}

	if (aContext == NULL) {
		aTrace->AddEvent(aTrace, "Context is NULL");
		FreeTraceAndReturn NULL;
	}
	IDataProvider* data = ProviderComposit_Cast(aContext->myDataProvider);
	size_t idCount = data->GetAllIdsCount(data);
	PersonId* ids = malloc(idCount * sizeof(PersonId));
	data->GetAllIds(data, ids);

	Person* result = calloc(1, sizeof(Person));
	size_t count = 0;
	int found = 0;
	for (size_t i = 0; i < idCount; i++) {
		result[count] = data->GetPerson(data, ids[i]);

		// TODO: handle error

		size_t matches = 0;

		if (aPrototype.title != NULL && strcmp(aPrototype.title, result[count].title) == 0) {
			matches++;
		}
		found = 0;
		for (size_t prototypeIndex = 0; prototypeIndex < aPrototype.firstNameCount;
			 prototypeIndex++) {
			for (size_t realIndex = 0; realIndex < result[count].firstNameCount; realIndex++) {
				if (strcmp(
						aPrototype.firstNames[prototypeIndex],
						result[count].firstNames[realIndex])
					== 0) {
					found = 1;
					break;
				}
			}
			if (found) {
				break;
			}
		}
		if (found) {
			matches++;
		}
		if (aPrototype.titleOfNobility != NULL
			&& strcmp(aPrototype.titleOfNobility, result[count].titleOfNobility) == 0) {
			matches++;
		}
		found = 0;
		for (size_t prototypeIndex = 0; prototypeIndex < aPrototype.lastNameCount;
			 prototypeIndex++) {
			for (size_t realIndex = 0; realIndex < result[count].lastNameCount; realIndex++) {
				if (strcmp(aPrototype.lastNames[prototypeIndex], result[count].lastNames[realIndex])
					== 0) {
					found = 1;
					break;
				}
			}
			if (found) {
				break;
			}
		}
		if (found) {
			matches++;
		}
		if (aPrototype.gender != NULL && strcmp(aPrototype.gender, result[count].gender) == 0) {
			matches++;
		}
		if (aPrototype.dateOfBirth != NULL
			&& strcmp(aPrototype.dateOfBirth, result[count].dateOfBirth) == 0) {
			matches++;
		}
		if (aPrototype.placeOfBirth != NULL
			&& strcmp(aPrototype.placeOfBirth, result[count].placeOfBirth) == 0) {
			matches++;
		}
		if (aPrototype.dateOfDeath != NULL
			&& strcmp(aPrototype.dateOfDeath, result[count].dateOfDeath) == 0) {
			matches++;
		}
		if (aPrototype.placeOfDeath != NULL
			&& strcmp(aPrototype.placeOfDeath, result[count].placeOfDeath) == 0) {
			matches++;
		}
		if (aPrototype.remark != NULL && strcmp(aPrototype.remark, result[count].remark) == 0) {
			matches++;
		}
		if (matches >= aMinMatches) {
			PopulateNullValues(result + count, aContext);
			count++;
			result = realloc(result, (count + 1) * sizeof(Person));
			continue;
		}
	}
	free(ids);

	free(aContext->myCreatedPersons);
	aContext->myCreatedPersons = result;

	*aOutPersonsCount = count;
	FreeTraceAndReturn result;
}

FT_API Relation*
GetPersonRelations(Context* aContext, PersonId aId, size_t* aOutRelationsCount, ITrace* aTrace) {
	int IsNoOpTrace = 0;
	if (aTrace == NULL || aTrace->CreateSubTrace == NULL || aTrace->AddEvent == NULL
		|| aTrace->Succeed == NULL || aTrace->Fail == NULL || aTrace->Free == NULL) {
		IsNoOpTrace = 1;
		aTrace = CreateNoOpTrace();
	}

	if (aContext == NULL) {
		aTrace->AddEvent(aTrace, "Context is NULL");
		FreeTraceAndReturn NULL;
	}
	IDataProvider* data = ProviderComposit_Cast(aContext->myDataProvider);
	size_t count = data->GetAllRelationsOfIdCount(data, aId);
	Relation* result = malloc(sizeof(Relation) * count);
	data->GetAllRelationsOfId(data, aId, result);

	for (size_t i = 0; i < count; i++) {
		if (result[i].relationship == NULL) {
			result[i].relationship = aContext->myDefaultString;
		}
		if (result[i].startDate == NULL) {
			result[i].startDate = aContext->myDefaultString;
		}
		if (result[i].endDate == NULL) {
			result[i].endDate = aContext->myDefaultString;
		}
	}

	*aOutRelationsCount = count;
	FreeTraceAndReturn result;
}

FT_API int
GetRelativeGeneration(Context* aContext, PersonId aRefId, PersonId aTargetId, ITrace* aTrace) {
	int IsNoOpTrace = 0;
	if (aTrace == NULL || aTrace->CreateSubTrace == NULL || aTrace->AddEvent == NULL
		|| aTrace->Succeed == NULL || aTrace->Fail == NULL || aTrace->Free == NULL) {
		IsNoOpTrace = 1;
		aTrace = CreateNoOpTrace();
	}

	if (aContext == NULL) {
		aTrace->AddEvent(aTrace, "Context is NULL");
		FreeTraceAndReturn INT_MIN;
	}

	FreeTraceAndReturn ComputeRelativeGeneration(aContext->myMedaData, aRefId, aTargetId);
}

FT_API int
IsDefaultString(Context* aContext, const char* aStringToCheck) {
	if (aContext == NULL) {
		return 0;
	}
	return aContext->myDefaultString == aStringToCheck ? 1 : 0;
}
