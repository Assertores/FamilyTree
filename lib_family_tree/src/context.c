#include "algorythms.h"
#include "family_tree/api.h"
#include "internal_types.h"
#include "patch.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define SET_TRACE()                                                                   \
	int IsNoOpTrace = 0;                                                              \
	if (aTrace == NULL || aTrace->CreateSubTrace == NULL || aTrace->AddEvent == NULL  \
		|| aTrace->Succeed == NULL || aTrace->Fail == NULL || aTrace->Free == NULL) { \
		IsNoOpTrace = 1;                                                              \
		aTrace = CreateNoOpTrace();                                                   \
	}

#define FREE_TRACE_AND_RETURN \
	if (IsNoOpTrace) {        \
		aTrace->Free(aTrace); \
	}                         \
	return

#define SUBTRACE(aName, aCode)                        \
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
	SET_TRACE()

	aTrace->AddEvent(aTrace, "Allocate Context");
	Context* context = calloc(1, sizeof(Context));
	if (context == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN NULL;
	}

	aTrace->AddEvent(aTrace, "Create Provider Composit");
	context->myDataProvider = CreateProviderComposit(aTrace);
	if (context->myDataProvider == NULL) {
		aTrace->Fail(aTrace, "Failed to Create Provider Composit");
		FREE_TRACE_AND_RETURN NULL;
	}
	context->myMedaData.myPersonCount = 0;
	context->myMedaData.myPersons = NULL;
	context->myDefaultString = "N/A";

	FREE_TRACE_AND_RETURN context;
}

FT_API void
Free(Context* aContext, ITrace* aTrace) {
	SET_TRACE()
	if (aContext == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN;
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

	FREE_TRACE_AND_RETURN;
}

FT_API Context*
CreateWithCSVAndJSON(const char* aPath, ITrace* aTrace) {
	SET_TRACE()

	ITrace* subtrace;
	SUBTRACE("Create", Context* context = Create(subtrace));
	if (context == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN context;
	}

	SUBTRACE(
		"CreateDefaultPlatform",
		IPlatform* platform = CreateDefaultPlatform(context, subtrace));
	SUBTRACE(
		"CreateCSVRelations",
		IRelationals* relations = CreateCSVRelations(context, aPath, platform, subtrace));
	SUBTRACE(
		"CreateJSONPersonals",
		IPersonals* personals = CreateJSONPersonals(context, aPath, platform, subtrace));
	SUBTRACE(
		"CreateDataProvider",
		IDataProvider* provider = CreateDataProvider(context, relations, personals, subtrace));
	SUBTRACE("AddDataProvider", AddDataProvider(context, provider, subtrace));

	FREE_TRACE_AND_RETURN context;
}

FT_API Context*
CreateCSVAndJSONWithIO(const char* aPath, IPlatform* aPlatform, ITrace* aTrace) {
	SET_TRACE()

	ITrace* subtrace;
	SUBTRACE("Create", Context* context = Create(subtrace));
	if (context == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN context;
	}

	SUBTRACE(
		"CreateCSVRelations",
		IRelationals* relations = CreateCSVRelations(context, aPath, aPlatform, subtrace));
	SUBTRACE(
		"CreateJSONPersonals",
		IPersonals* personals = CreateJSONPersonals(context, aPath, aPlatform, subtrace));
	SUBTRACE(
		"CreateDataProvider",
		IDataProvider* provider = CreateDataProvider(context, relations, personals, subtrace));
	SUBTRACE("AddDataProvider", AddDataProvider(context, provider, subtrace));

	FREE_TRACE_AND_RETURN context;
}

FT_API void
AddDataProvider(Context* aContext, IDataProvider* aData, ITrace* aTrace) {
	SET_TRACE()

	if (aContext == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN;
	}
	if (aData == NULL) {
		aTrace->Fail(aTrace, "aData is NULL");
		FREE_TRACE_AND_RETURN;
	}
	if (aData->Copy == NULL) {
		aTrace->Fail(aTrace, "aData has no Copy");
		FREE_TRACE_AND_RETURN;
	}
	if (aData->GetAllIds == NULL) {
		aTrace->Fail(aTrace, "aData has no GetAllIds");
		FREE_TRACE_AND_RETURN;
	}
	if (aData->GetAllIdsCount == NULL) {
		aTrace->Fail(aTrace, "aData has no GetAllIdsCount");
		FREE_TRACE_AND_RETURN;
	}
	if (aData->GetAllRelationsOfId == NULL) {
		aTrace->Fail(aTrace, "aData has no GetAllRelationsOfId");
		FREE_TRACE_AND_RETURN;
	}
	if (aData->GetAllRelationsOfIdCount == NULL) {
		aTrace->Fail(aTrace, "aData has no GetAllRelationsOfIdCount");
		FREE_TRACE_AND_RETURN;
	}
	if (aData->GetPerson == NULL) {
		aTrace->Fail(aTrace, "aData has no GetPerson");
		FREE_TRACE_AND_RETURN;
	}
	if (aData->PlayPerson == NULL) {
		aTrace->Fail(aTrace, "aData has no PlayPerson");
		FREE_TRACE_AND_RETURN;
	}

	aTrace->AddEvent(aTrace, "add Copied aData to composit.");
	ProviderComposit_AddDataProvider(aContext->myDataProvider, aData->Copy(aData, aTrace), aTrace);

	aTrace->AddEvent(aTrace, "Recalculate MedaData for Graph");
	FreeMetaData(&aContext->myMedaData);
	aContext->myMedaData = CreateMetaData(ProviderComposit_Cast(aContext->myDataProvider), aTrace);

	FREE_TRACE_AND_RETURN;
}

FT_API IDataProvider*
CreateDataProvider(
	Context* aContext,
	IRelationals* aRelations,
	IPersonals* aPersonals,
	ITrace* aTrace) {
	SET_TRACE()

	if (aContext == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aRelations == NULL) {
		aTrace->Fail(aTrace, "aRelations is NULL");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aRelations->Copy == NULL) {
		aTrace->Fail(aTrace, "aRelations has no Copy");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aRelations->Free == NULL) {
		aTrace->Fail(aTrace, "aRelations has no Free");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aRelations->GetAllIds == NULL) {
		aTrace->Fail(aTrace, "aRelations has no GetAllIds");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aRelations->GetAllIdsCount == NULL) {
		aTrace->Fail(aTrace, "aRelations has no GetAllIdsCount");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aRelations->GetAllRelationsOfId == NULL) {
		aTrace->Fail(aTrace, "aRelations has no GetAllRelationsOfId");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aRelations->GetAllRelationsOfIdCount == NULL) {
		aTrace->Fail(aTrace, "aRelations has no GetAllRelationsOfIdCount");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aPersonals == NULL) {
		aTrace->Fail(aTrace, "aPersonals is NULL");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aPersonals->Copy == NULL) {
		aTrace->Fail(aTrace, "aPersonals has no Copy");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aPersonals->Free == NULL) {
		aTrace->Fail(aTrace, "aPersonals has no Free");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aPersonals->GetAllIds == NULL) {
		aTrace->Fail(aTrace, "aPersonals has no GetAllIds");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aPersonals->GetAllIdsCount == NULL) {
		aTrace->Fail(aTrace, "aPersonals has no GetAllIdsCount");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aPersonals->GetPerson == NULL) {
		aTrace->Fail(aTrace, "aPersonals has no GetPerson");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aPersonals->PlayPerson == NULL) {
		aTrace->Fail(aTrace, "aPersonals has no PlayPerson");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aPersonals->ShowImages == NULL) {
		aTrace->Fail(aTrace, "aPersonals has no ShowImages");
		FREE_TRACE_AND_RETURN NULL;
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
		FREE_TRACE_AND_RETURN NULL;
	}

	aTrace->AddEvent(aTrace, "Keep track of constructed objects.");
	IDataProvider** newArray = realloc(
		aContext->myCreatedDataProviders,
		sizeof(IRelationals*) * (aContext->myCreatedDataProvidersSize + 1));
	if (newArray == NULL) {
		aTrace->Fail(aTrace, "Unable to realloc array");
		data->Free(data, aTrace);
		free(data);
		FREE_TRACE_AND_RETURN NULL;
	}

	newArray[aContext->myCreatedDataProvidersSize] = data;
	aContext->myCreatedDataProvidersSize++;
	aContext->myCreatedDataProviders = newArray;

	FREE_TRACE_AND_RETURN data;
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
	SET_TRACE()

	if (aContext == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (!CheckIo(aPlatform)) {
		aTrace->Fail(aTrace, "aPlatform is malformed");
		FREE_TRACE_AND_RETURN NULL;
	}

	aTrace->AddEvent(aTrace, "Create Shared CSV Relation");
	IRelationals* relations = CreateSharedRelationalsDecorator(
		CreateCSVRelation(aPath, aPlatform->Copy(aPlatform, aTrace), aTrace),
		aTrace);
	if (relations == NULL) {
		aTrace->Fail(aTrace, "Unable to Create CSV Relation");
		FREE_TRACE_AND_RETURN NULL;
	}

	aTrace->AddEvent(aTrace, "Keep track of constructed objects.");
	IRelationals** newArray = realloc(
		aContext->myCreatedRelations,
		sizeof(IRelationals*) * (aContext->myCreatedRelationsSize + 1));
	if (newArray == NULL) {
		aTrace->Fail(aTrace, "Unable to realloc array");
		relations->Free(relations, aTrace);
		free(relations);
		FREE_TRACE_AND_RETURN NULL;
	}

	newArray[aContext->myCreatedRelationsSize] = relations;
	aContext->myCreatedRelationsSize++;
	aContext->myCreatedRelations = newArray;
	FREE_TRACE_AND_RETURN relations;
}

FT_API IPersonals*
CreateJSONPersonals(Context* aContext, const char* aPath, IPlatform* aPlatform, ITrace* aTrace) {
	SET_TRACE()

	if (aContext == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (!CheckIo(aPlatform)) {
		aTrace->Fail(aTrace, "aPlatform is malformed");
		FREE_TRACE_AND_RETURN NULL;
	}

	aTrace->AddEvent(aTrace, "Create Shared JSON Personals");
	IPersonals* persons = CreateSharedPersonalsDecorator(
		CreateJSONPerson(aPath, aPlatform->Copy(aPlatform, aTrace), aTrace),
		aTrace);
	if (persons == NULL) {
		aTrace->Fail(aTrace, "Unable to Create JSON Personals");
		FREE_TRACE_AND_RETURN NULL;
	}

	aTrace->AddEvent(aTrace, "Keep track of constructed objects.");
	IPersonals** newArray = realloc(
		aContext->myCreatedPersonals,
		sizeof(IRelationals*) * (aContext->myCreatedPersonalsSize + 1));
	if (newArray == NULL) {
		aTrace->Fail(aTrace, "Unable to realloc array");
		persons->Free(persons, aTrace);
		free(persons);
		FREE_TRACE_AND_RETURN NULL;
	}

	newArray[aContext->myCreatedPersonalsSize] = persons;
	aContext->myCreatedPersonalsSize++;
	aContext->myCreatedPersonals = newArray;
	FREE_TRACE_AND_RETURN persons;
}

FT_API IPlatform*
CreateDefaultPlatform(Context* aContext, ITrace* aTrace) {
	SET_TRACE()

	if (aContext == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN NULL;
	}

	aTrace->AddEvent(aTrace, "Create Shared Default Platform");
	IPlatform* platform = CreateSharedPlatformDecorator(CreatePlatform(aTrace), aTrace);
	if (platform == NULL) {
		aTrace->Fail(aTrace, "Unable to Create Platform");
		FREE_TRACE_AND_RETURN NULL;
	}

	aTrace->AddEvent(aTrace, "Keep track of constructed objects.");
	IPlatform** newArray = realloc(
		aContext->myCreatedPlatforms,
		sizeof(IRelationals*) * (aContext->myCreatedPlatformSize + 1));
	if (newArray == NULL) {
		aTrace->Fail(aTrace, "Unable to realloc array");
		platform->Free(platform, aTrace);
		free(platform);
		FREE_TRACE_AND_RETURN NULL;
	}

	newArray[aContext->myCreatedPlatformSize] = platform;
	aContext->myCreatedPlatformSize++;
	aContext->myCreatedPlatforms = newArray;
	FREE_TRACE_AND_RETURN platform;
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
	if (aPerson->professions == NULL) {
		aPerson->professions = &aContext->myDefaultString;
	}
	for (int i = 0; i < aPerson->placeOfResidenceCount; i++) {
		if (aPerson->placeOfResidences[i].name == NULL) {
			aPerson->placeOfResidences[i].name = aContext->myDefaultString;
		}
		if (aPerson->placeOfResidences[i].startDate == NULL) {
			aPerson->placeOfResidences[i].startDate = aContext->myDefaultString;
		}
		if (aPerson->placeOfResidences[i].endDate == NULL) {
			aPerson->placeOfResidences[i].endDate = aContext->myDefaultString;
		}
	}
}

FT_API Person
GetPerson(Context* aContext, PersonId aId, ITrace* aTrace) {
	SET_TRACE()

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
		FREE_TRACE_AND_RETURN p;
	}

	IDataProvider* interface = ProviderComposit_Cast(aContext->myDataProvider);
	p = interface->GetPerson(interface, aId, aTrace);

	aTrace->AddEvent(aTrace, "Populate Null Values");
	PopulateNullValues(&p, aContext);
	FREE_TRACE_AND_RETURN p;
}

FT_API void
PlayPerson(Context* aContext, PersonId aId, ITrace* aTrace) {
	SET_TRACE()

	if (aContext == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN;
	}

	IDataProvider* interface = ProviderComposit_Cast(aContext->myDataProvider);
	interface->PlayPerson(interface, aId, aTrace);
	FREE_TRACE_AND_RETURN;
}

FT_API void
ShowImagesOfPerson(Context* aContext, PersonId aId, ITrace* aTrace) {
	SET_TRACE()

	if (aContext == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN;
	}

	IDataProvider* interface = ProviderComposit_Cast(aContext->myDataProvider);
	interface->ShowImages(interface, aId, aTrace);
	FREE_TRACE_AND_RETURN;
}

FT_API Person*
GetPersonsMatchingPattern(
	Context* aContext,
	Person aPrototype,
	size_t aMinMatches,
	size_t* aOutPersonsCount,
	ITrace* aTrace) {
	SET_TRACE()

	if (aContext == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN NULL;
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
		PopulateNullValues(result + count, aContext);
		ITrace* subtrace = aTrace->CreateSubTrace(aTrace, result[count].firstNames[0]);

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

	FREE_TRACE_AND_RETURN result;
}

FT_API Relation*
GetPersonRelations(Context* aContext, PersonId aId, size_t* aOutRelationsCount, ITrace* aTrace) {
	SET_TRACE()

	if (aContext == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN NULL;
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
	FREE_TRACE_AND_RETURN result;
}

FT_API int
GetRelativeGeneration(Context* aContext, PersonId aRefId, PersonId aTargetId, ITrace* aTrace) {
	SET_TRACE()

	if (aContext == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN INT_MIN;
	}

	int result = ComputeRelativeGeneration(aContext->myMedaData, aRefId, aTargetId, aTrace);

	FREE_TRACE_AND_RETURN result;
}

FT_API PersonId*
GetPartners(Context* aContext, PersonId aId, size_t* aOutParterCount, ITrace* aTrace) {
	SET_TRACE()

	if (aContext == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aOutParterCount == NULL) {
		aTrace->Fail(aTrace, "Parter Count is NULL");
		FREE_TRACE_AND_RETURN NULL;
	}

	size_t minCount = ComputePartnersMinimalCount(aContext->myMedaData, aId, aTrace);
	if (minCount == 0) {
		*aOutParterCount = 0;
		aTrace->Succeed(aTrace);
		FREE_TRACE_AND_RETURN NULL;
	}

	PersonId* result = calloc(minCount, sizeof(PersonId));
	minCount = ComputePartners(aContext->myMedaData, aId, result, aTrace);
	if (minCount == 0) {
		*aOutParterCount = 0;
		aTrace->Succeed(aTrace);
		FREE_TRACE_AND_RETURN NULL;
	}

	result = realloc(result, minCount * sizeof(PersonId));
	// TODO: keep track of memory

	*aOutParterCount = minCount;
	FREE_TRACE_AND_RETURN result;
}

FT_API PersonId*
GetSiblings(Context* aContext, PersonId aId, size_t* aOutSiblingCount, ITrace* aTrace) {
	SET_TRACE()

	if (aContext == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aOutSiblingCount == NULL) {
		aTrace->Fail(aTrace, "Sibling Count is NULL");
		FREE_TRACE_AND_RETURN NULL;
	}

	size_t minCount = ComputeSiblingsMinimalCount(aContext->myMedaData, aId, aTrace);
	if (minCount == 0) {
		*aOutSiblingCount = 0;
		aTrace->Succeed(aTrace);
		FREE_TRACE_AND_RETURN NULL;
	}

	PersonId* result = calloc(minCount, sizeof(PersonId));
	minCount = ComputeSiblings(aContext->myMedaData, aId, result, aTrace);
	if (minCount == 0) {
		*aOutSiblingCount = 0;
		aTrace->Succeed(aTrace);
		FREE_TRACE_AND_RETURN NULL;
	}

	result = realloc(result, minCount * sizeof(PersonId));
	// TODO: keep track of memory

	*aOutSiblingCount = minCount;
	FREE_TRACE_AND_RETURN result;
}

FT_API PersonId*
GetCommonParents(
	Context* aContext,
	size_t aIdCount,
	PersonId* aIds,
	size_t* aOutParentCount,
	ITrace* aTrace) {
	SET_TRACE()

	if (aContext == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aOutParentCount == NULL) {
		aTrace->Fail(aTrace, "Parent Count is NULL");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aIdCount == 0) {
		*aOutParentCount = 0;
		aTrace->Succeed(aTrace);
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aIds == NULL) {
		aTrace->Fail(aTrace, "Ids is NULL");
		FREE_TRACE_AND_RETURN NULL;
	}

	size_t count = GetParentsCount(aContext->myMedaData, aIds[0], aTrace);
	PersonId* result = calloc(count, sizeof(PersonId));
	GetParents(aContext->myMedaData, aIds[0], result, aTrace);
	for (size_t i = 1; i < aIdCount; i++) {
		count = IntersectParents(aContext->myMedaData, aIds[i], count, result, aTrace);
	}
	result = realloc(result, count * sizeof(PersonId));
	// TODO: keep track of memory

	*aOutParentCount = count;
	FREE_TRACE_AND_RETURN result;
}

FT_API PersonId*
GetCommonChildren(
	Context* aContext,
	size_t aIdCount,
	PersonId* aIds,
	size_t* aOutChildrenCount,
	ITrace* aTrace) {
	SET_TRACE()

	if (aContext == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aOutChildrenCount == NULL) {
		aTrace->Fail(aTrace, "Children Count is NULL");
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aIdCount == 0) {
		*aOutChildrenCount = 0;
		aTrace->Succeed(aTrace);
		FREE_TRACE_AND_RETURN NULL;
	}
	if (aIds == NULL) {
		aTrace->Fail(aTrace, "Ids is NULL");
		FREE_TRACE_AND_RETURN NULL;
	}

	size_t count = GetChildrenCount(aContext->myMedaData, aIds[0], aTrace);
	PersonId* result = calloc(count, sizeof(PersonId));
	GetChildrens(aContext->myMedaData, aIds[0], result, aTrace);
	for (size_t i = 1; i < aIdCount; i++) {
		count = IntersectChildrens(aContext->myMedaData, aIds[i], count, result, aTrace);
	}
	result = realloc(result, count * sizeof(PersonId));
	// TODO: keep track of memory

	*aOutChildrenCount = count;
	FREE_TRACE_AND_RETURN result;
}

// 1 if id was unique otherwise 0
int
AddIfUnique(PersonId** aArray, size_t aCount, PersonId aNewId) {
	for (size_t i = 0; i < aCount; i++) {
		if ((*aArray)[i] == aNewId) {
			return 0;
		}
	}
	*aArray = realloc(*aArray, (aCount + 1) * sizeof(PersonId));
	(*aArray)[aCount] = aNewId;
	return 1;
}

FT_API void
BuildGraph(
	Context* aContext,
	PersonId aId,
	size_t aDistance,
	GraphBuilderStrategy* aStrategy,
	ITrace* aTrace) {
	SET_TRACE()

	if (aContext == NULL) {
		aTrace->Fail(aTrace, "Context is NULL");
		FREE_TRACE_AND_RETURN;
	}
	if (aStrategy == NULL) {
		aTrace->Fail(aTrace, "Strategy is NULL");
		FREE_TRACE_AND_RETURN;
	}
	if (aStrategy->FamilieStrategy == NULL) {
		aTrace->Fail(aTrace, "Strategy does not contain a FamilieStrategy method");
		FREE_TRACE_AND_RETURN;
	}
	if (aStrategy->PersonInGenerationStrategy == NULL) {
		aTrace->Fail(aTrace, "Strategy does not contain a PersonInGenerationStrategy method");
		FREE_TRACE_AND_RETURN;
	}

	PersonId* personToShowSet = malloc(sizeof(PersonId));
	personToShowSet[0] = aId;
	size_t personToShowCount = 1;
	int wasUnique = 0;
	int othersGeneration = 0;

	aTrace->AddEvent(aTrace, "Send requested person id into Strategy");
	aStrategy->PersonInGenerationStrategy(aStrategy, aId, 0);

	ITrace* realTrace = aTrace;
	aTrace = realTrace->CreateSubTrace(realTrace, "Find unique persons");
	ITrace* subtrace;
	for (size_t currentDistance = 0; currentDistance < aDistance; currentDistance++) {
		aTrace->AddEvent(aTrace, "search one distance futher away");
		int newPersonCount = 0;
		for (size_t i = 0; i < personToShowCount; i++) {
			size_t count = 0;

			SUBTRACE(
				"Get relations",
				Relation* array =
					GetPersonRelations(aContext, personToShowSet[i], &count, subtrace));
			for (size_t arrI = 0; arrI < count; arrI++) {
				wasUnique = AddIfUnique(
					&personToShowSet,
					personToShowCount + newPersonCount,
					array[arrI].id1);
				if (wasUnique != 0) {
					aTrace->AddEvent(aTrace, "found a new person");
					newPersonCount++;

					SUBTRACE(
						"Get Generation for person",
						othersGeneration =
							GetRelativeGeneration(aContext, aId, array[arrI].id1, subtrace));

					aTrace->AddEvent(aTrace, "Send person into Strategy");
					aStrategy->PersonInGenerationStrategy(
						aStrategy,
						array[arrI].id1,
						othersGeneration);
				}
				wasUnique = AddIfUnique(
					&personToShowSet,
					personToShowCount + newPersonCount,
					array[arrI].id2);
				if (wasUnique != 0) {
					aTrace->AddEvent(aTrace, "found a new person");
					newPersonCount++;

					SUBTRACE(
						"Get Generation for person",
						othersGeneration =
							GetRelativeGeneration(aContext, aId, array[arrI].id2, aTrace));

					aTrace->AddEvent(aTrace, "Send person into Strategy");
					aStrategy->PersonInGenerationStrategy(
						aStrategy,
						array[arrI].id2,
						othersGeneration);
				}
			}
		}
		personToShowCount += newPersonCount;
	}
	aTrace->Free(aTrace);

	aTrace = realTrace->CreateSubTrace(realTrace, "Generate Families");
	for (size_t personIndex = 0; personIndex < personToShowCount; personIndex++) {
		size_t count = 0;
		SUBTRACE(
			"Find Partners",
			PersonId* partners =
				GetPartners(aContext, personToShowSet[personIndex], &count, subtrace));
		for (size_t i = 0; i < count; i++) {
			PersonId parents[] = {personToShowSet[personIndex], partners[i]};
			size_t childCount = 0;
			SUBTRACE(
				"Get Childrens of two parents",
				PersonId* children = GetCommonChildren(aContext, 2, parents, &childCount, aTrace));

			size_t parentCount = 0;
			SUBTRACE(
				"Get All Parents of the Familie",
				PersonId* fullParents =
					GetCommonParents(aContext, childCount, children, &parentCount, aTrace));

			aTrace->AddEvent(aTrace, "Send familie into Strategy");
			aStrategy->FamilieStrategy(aStrategy, fullParents, parentCount, children, childCount);
		}
	}
	aTrace->Free(aTrace);

	aTrace = realTrace;
	FREE_TRACE_AND_RETURN;
}

FT_API int
IsDefaultString(Context* aContext, const char* aStringToCheck) {
	if (aContext == NULL) {
		return 0;
	}
	return aContext->myDefaultString == aStringToCheck ? 1 : 0;
}
