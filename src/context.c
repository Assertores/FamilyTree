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

	aTrace->AddEvent(aTrace, "Allocate Context");
	Context* context = calloc(1, sizeof(Context));
	if (context == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FreeTraceAndReturn NULL;
	}

	aTrace->AddEvent(aTrace, "Create Provider Composit");
	context->myDataProvider = CreateProviderComposit(aTrace);
	if (context->myDataProvider == NULL) {
		aTrace->Fail(aTrace, "Failed to Create Provider Composit");
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
		aTrace->Fail(aTrace, "Context is NULL");
		FreeTraceAndReturn;
	}

	aTrace->AddEvent(aTrace, "Free DataProvider");
	IDataProvider* interface = ProviderComposit_Cast(aContext->myDataProvider);
	interface->Free(interface, aTrace);

	aTrace->AddEvent(aTrace, "Free own DataProvider");
	for (size_t i = 0; i < aContext->myCreatedDataProvidersSize; i++) {
		aContext->myCreatedDataProviders[i]->Free(aContext->myCreatedDataProviders[i], aTrace);
	}
	free(aContext->myCreatedDataProviders);

	aTrace->AddEvent(aTrace, "Free own Relations");
	for (size_t i = 0; i < aContext->myCreatedRelationsSize; i++) {
		aContext->myCreatedRelations[i]->Free(aContext->myCreatedRelations[i], aTrace);
	}
	free(aContext->myCreatedRelations);

	aTrace->AddEvent(aTrace, "Free own Personals");
	for (size_t i = 0; i < aContext->myCreatedPersonalsSize; i++) {
		aContext->myCreatedPersonals[i]->Free(aContext->myCreatedPersonals[i], aTrace);
	}
	free(aContext->myCreatedPersonals);

	aTrace->AddEvent(aTrace, "Free own Platforms");
	for (size_t i = 0; i < aContext->myCreatedPlatformSize; i++) {
		aContext->myCreatedPlatforms[i]->Free(aContext->myCreatedPlatforms[i], aTrace);
	}
	free(aContext->myCreatedPlatforms);

	aTrace->AddEvent(aTrace, "Free Person array");
	free(aContext->myCreatedPersons);
	aTrace->AddEvent(aTrace, "Free Meta data for graph");
	FreeMetaData(&aContext->myMedaData);

	aTrace->AddEvent(aTrace, "Free context");
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
		aTrace->Fail(aTrace, "Context is NULL");
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
		aTrace->Fail(aTrace, "Context is NULL");
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
		aTrace->Fail(aTrace, "Context is NULL");
		FreeTraceAndReturn;
	}
	if (aData == NULL) {
		aTrace->Fail(aTrace, "aData is NULL");
		FreeTraceAndReturn;
	}
	if (aData->Copy == NULL) {
		aTrace->Fail(aTrace, "aData has no Copy");
		FreeTraceAndReturn;
	}
	if (aData->GetAllIds == NULL) {
		aTrace->Fail(aTrace, "aData has no GetAllIds");
		FreeTraceAndReturn;
	}
	if (aData->GetAllIdsCount == NULL) {
		aTrace->Fail(aTrace, "aData has no GetAllIdsCount");
		FreeTraceAndReturn;
	}
	if (aData->GetAllRelationsOfId == NULL) {
		aTrace->Fail(aTrace, "aData has no GetAllRelationsOfId");
		FreeTraceAndReturn;
	}
	if (aData->GetAllRelationsOfIdCount == NULL) {
		aTrace->Fail(aTrace, "aData has no GetAllRelationsOfIdCount");
		FreeTraceAndReturn;
	}
	if (aData->GetPerson == NULL) {
		aTrace->Fail(aTrace, "aData has no GetPerson");
		FreeTraceAndReturn;
	}
	if (aData->PlayPerson == NULL) {
		aTrace->Fail(aTrace, "aData has no PlayPerson");
		FreeTraceAndReturn;
	}

	aTrace->AddEvent(aTrace, "add Copied aData to composit.");
	ProviderComposit_AddDataProvider(aContext->myDataProvider, aData->Copy(aData, aTrace), aTrace);

	aTrace->AddEvent(aTrace, "Recalculate MedaData for Graph");
	FreeMetaData(&aContext->myMedaData);
	aContext->myMedaData = CreateMetaData(ProviderComposit_Cast(aContext->myDataProvider), aTrace);

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
		aTrace->Fail(aTrace, "Context is NULL");
		FreeTraceAndReturn NULL;
	}
	if (aRelations == NULL) {
		aTrace->Fail(aTrace, "aRelations is NULL");
		FreeTraceAndReturn NULL;
	}
	if (aRelations->Copy == NULL) {
		aTrace->Fail(aTrace, "aRelations has no Copy");
		FreeTraceAndReturn NULL;
	}
	if (aRelations->Free == NULL) {
		aTrace->Fail(aTrace, "aRelations has no Free");
		FreeTraceAndReturn NULL;
	}
	if (aRelations->GetAllIds == NULL) {
		aTrace->Fail(aTrace, "aRelations has no GetAllIds");
		FreeTraceAndReturn NULL;
	}
	if (aRelations->GetAllIdsCount == NULL) {
		aTrace->Fail(aTrace, "aRelations has no GetAllIdsCount");
		FreeTraceAndReturn NULL;
	}
	if (aRelations->GetAllRelationsOfId == NULL) {
		aTrace->Fail(aTrace, "aRelations has no GetAllRelationsOfId");
		FreeTraceAndReturn NULL;
	}
	if (aRelations->GetAllRelationsOfIdCount == NULL) {
		aTrace->Fail(aTrace, "aRelations has no GetAllRelationsOfIdCount");
		FreeTraceAndReturn NULL;
	}
	if (aPersonals == NULL) {
		aTrace->Fail(aTrace, "aPersonals is NULL");
		FreeTraceAndReturn NULL;
	}
	if (aPersonals->Copy == NULL) {
		aTrace->Fail(aTrace, "aPersonals has no Copy");
		FreeTraceAndReturn NULL;
	}
	if (aPersonals->Free == NULL) {
		aTrace->Fail(aTrace, "aPersonals has no Free");
		FreeTraceAndReturn NULL;
	}
	if (aPersonals->GetAllIds == NULL) {
		aTrace->Fail(aTrace, "aPersonals has no GetAllIds");
		FreeTraceAndReturn NULL;
	}
	if (aPersonals->GetAllIdsCount == NULL) {
		aTrace->Fail(aTrace, "aPersonals has no GetAllIdsCount");
		FreeTraceAndReturn NULL;
	}
	if (aPersonals->GetPerson == NULL) {
		aTrace->Fail(aTrace, "aPersonals has no GetPerson");
		FreeTraceAndReturn NULL;
	}
	if (aPersonals->PlayPerson == NULL) {
		aTrace->Fail(aTrace, "aPersonals has no PlayPerson");
		FreeTraceAndReturn NULL;
	}
	if (aPersonals->ShowImages == NULL) {
		aTrace->Fail(aTrace, "aPersonals has no ShowImages");
		FreeTraceAndReturn NULL;
	}

	aTrace->AddEvent(aTrace, "Create Shared Forwarding Provider");
	IDataProvider* data = CreateSharedDataProviderDecorator(
		CreateForwardingProvider(
			aRelations->Copy(aRelations, aTrace),
			aPersonals->Copy(aPersonals, aTrace),
			aTrace),
		aTrace);
	if (data == NULL) {
		aTrace->Fail(aTrace, "Unable to Create Data Provider");
		FreeTraceAndReturn NULL;
	}

	aTrace->AddEvent(aTrace, "Keep track of constructed objects.");
	IDataProvider** newArray = realloc(
		aContext->myCreatedDataProviders,
		sizeof(IRelationals*) * (aContext->myCreatedDataProvidersSize + 1));
	if (newArray == NULL) {
		aTrace->Fail(aTrace, "Unable to realloc array");
		data->Free(data, aTrace);
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
		aTrace->Fail(aTrace, "Context is NULL");
		FreeTraceAndReturn NULL;
	}
	if (!CheckIo(aPlatform)) {
		aTrace->Fail(aTrace, "aPlatform is malformed");
		FreeTraceAndReturn NULL;
	}

	aTrace->AddEvent(aTrace, "Create Shared CSV Relation");
	IRelationals* relations = CreateSharedRelationalsDecorator(
		CreateCSVRelation(aPath, aPlatform->Copy(aPlatform, aTrace), aTrace),
		aTrace);
	if (relations == NULL) {
		aTrace->Fail(aTrace, "Unable to Create CSV Relation");
		FreeTraceAndReturn NULL;
	}

	aTrace->AddEvent(aTrace, "Keep track of constructed objects.");
	IRelationals** newArray = realloc(
		aContext->myCreatedRelations,
		sizeof(IRelationals*) * (aContext->myCreatedRelationsSize + 1));
	if (newArray == NULL) {
		aTrace->Fail(aTrace, "Unable to realloc array");
		relations->Free(relations, aTrace);
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
		aTrace->Fail(aTrace, "Context is NULL");
		FreeTraceAndReturn NULL;
	}
	if (!CheckIo(aPlatform)) {
		aTrace->Fail(aTrace, "aPlatform is malformed");
		FreeTraceAndReturn NULL;
	}

	aTrace->AddEvent(aTrace, "Create Shared JSON Personals");
	IPersonals* persons = CreateSharedPersonalsDecorator(
		CreateJSONPerson(aPath, aPlatform->Copy(aPlatform, aTrace), aTrace),
		aTrace);
	if (persons == NULL) {
		aTrace->Fail(aTrace, "Unable to Create JSON Personals");
		FreeTraceAndReturn NULL;
	}

	aTrace->AddEvent(aTrace, "Keep track of constructed objects.");
	IPersonals** newArray = realloc(
		aContext->myCreatedPersonals,
		sizeof(IRelationals*) * (aContext->myCreatedPersonalsSize + 1));
	if (newArray == NULL) {
		aTrace->Fail(aTrace, "Unable to realloc array");
		persons->Free(persons, aTrace);
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
		aTrace->Fail(aTrace, "Context is NULL");
		FreeTraceAndReturn NULL;
	}

	aTrace->AddEvent(aTrace, "Create Shared Default Platform");
	IPlatform* platform = CreateSharedPlatformDecorator(CreatePlatform(aTrace), aTrace);
	if (platform == NULL) {
		aTrace->Fail(aTrace, "Unable to Create Platform");
		FreeTraceAndReturn NULL;
	}

	aTrace->AddEvent(aTrace, "Keep track of constructed objects.");
	IPlatform** newArray = realloc(
		aContext->myCreatedPlatforms,
		sizeof(IRelationals*) * (aContext->myCreatedPlatformSize + 1));
	if (newArray == NULL) {
		aTrace->Fail(aTrace, "Unable to realloc array");
		platform->Free(platform, aTrace);
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
		aTrace->Fail(aTrace, "Context is NULL");
		FreeTraceAndReturn p;
	}

	IDataProvider* interface = ProviderComposit_Cast(aContext->myDataProvider);
	p = interface->GetPerson(interface, aId, aTrace);

	aTrace->AddEvent(aTrace, "Populate Null Values");
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
		aTrace->Fail(aTrace, "Context is NULL");
		FreeTraceAndReturn;
	}

	IDataProvider* interface = ProviderComposit_Cast(aContext->myDataProvider);
	interface->PlayPerson(interface, aId, aTrace);
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
		aTrace->Fail(aTrace, "Context is NULL");
		FreeTraceAndReturn;
	}

	IDataProvider* interface = ProviderComposit_Cast(aContext->myDataProvider);
	interface->ShowImages(interface, aId, aTrace);
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
		aTrace->Fail(aTrace, "Context is NULL");
		FreeTraceAndReturn NULL;
	}
	IDataProvider* data = ProviderComposit_Cast(aContext->myDataProvider);

	aTrace->AddEvent(aTrace, "Retreave all Ids");
	size_t idCount = data->GetAllIdsCount(data, aTrace);
	PersonId* ids = malloc(idCount * sizeof(PersonId));
	data->GetAllIds(data, ids, aTrace);

	Person* result = calloc(1, sizeof(Person));
	size_t count = 0;
	int found = 0;
	for (size_t i = 0; i < idCount; i++) {
		result[count] = data->GetPerson(data, ids[i], aTrace);
		ITrace* subtrace = aTrace->CreateSubTrace(subtrace, result[count].firstNames[0]);

		// TODO: handle error

		size_t matches = 0;

		if (aPrototype.title != NULL && strcmp(aPrototype.title, result[count].title) == 0) {
			subtrace->AddEvent(subtrace, "Title matches");
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
					subtrace->AddEvent(subtrace, "Firstname matches");
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
			subtrace->AddEvent(subtrace, "Title of Nobility matches");
			matches++;
		}
		found = 0;
		for (size_t prototypeIndex = 0; prototypeIndex < aPrototype.lastNameCount;
			 prototypeIndex++) {
			for (size_t realIndex = 0; realIndex < result[count].lastNameCount; realIndex++) {
				if (strcmp(aPrototype.lastNames[prototypeIndex], result[count].lastNames[realIndex])
					== 0) {
					subtrace->AddEvent(subtrace, "Lastname matches");
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
			subtrace->AddEvent(subtrace, "Gender matches");
			matches++;
		}
		if (aPrototype.dateOfBirth != NULL
			&& strcmp(aPrototype.dateOfBirth, result[count].dateOfBirth) == 0) {
			subtrace->AddEvent(subtrace, "date of birth matches");
			matches++;
		}
		if (aPrototype.placeOfBirth != NULL
			&& strcmp(aPrototype.placeOfBirth, result[count].placeOfBirth) == 0) {
			subtrace->AddEvent(subtrace, "place of birth matches");
			matches++;
		}
		if (aPrototype.dateOfDeath != NULL
			&& strcmp(aPrototype.dateOfDeath, result[count].dateOfDeath) == 0) {
			subtrace->AddEvent(subtrace, "date of death matches");
			matches++;
		}
		if (aPrototype.placeOfDeath != NULL
			&& strcmp(aPrototype.placeOfDeath, result[count].placeOfDeath) == 0) {
			subtrace->AddEvent(subtrace, "place of death matches");
			matches++;
		}
		if (aPrototype.remark != NULL && strcmp(aPrototype.remark, result[count].remark) == 0) {
			subtrace->AddEvent(subtrace, "remark matches");
			matches++;
		}
		if (matches >= aMinMatches) {
			subtrace->AddEvent(subtrace, "minimal matches reached");
			subtrace->AddEvent(subtrace, "Populate Null Values");
			PopulateNullValues(result + count, aContext);
			count++;

			subtrace->AddEvent(subtrace, "reallocate result array");
			result = realloc(result, (count + 1) * sizeof(Person));
		}
		subtrace->Free(subtrace);
	}
	free(ids);

	aTrace->AddEvent(aTrace, "store result for later free");
	free(aContext->myCreatedPersons);
	aContext->myCreatedPersons = result;

	aTrace->AddEvent(aTrace, "set output variable");
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
		aTrace->Fail(aTrace, "Context is NULL");
		FreeTraceAndReturn NULL;
	}
	IDataProvider* data = ProviderComposit_Cast(aContext->myDataProvider);
	size_t count = data->GetAllRelationsOfIdCount(data, aId, aTrace);
	Relation* result = malloc(sizeof(Relation) * count);
	data->GetAllRelationsOfId(data, aId, result, aTrace);

	aTrace->AddEvent(aTrace, "Populate Null Values");
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

	aTrace->AddEvent(aTrace, "set output variable");
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
		aTrace->Fail(aTrace, "Context is NULL");
		FreeTraceAndReturn INT_MIN;
	}

	FreeTraceAndReturn ComputeRelativeGeneration(aContext->myMedaData, aRefId, aTargetId, aTrace);
}

FT_API int
IsDefaultString(Context* aContext, const char* aStringToCheck) {
	if (aContext == NULL) {
		return 0;
	}
	return aContext->myDefaultString == aStringToCheck ? 1 : 0;
}
