#include "algorythms.h"
#include "family_tree/api.h"
#include "internal_types.h"
#include "patch.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

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
Create() {
	Context* context = calloc(1, sizeof(Context));
	if (context == NULL) {
		return NULL;
	}
	context->myDataProvider = CreateProviderComposit();
	if (context->myDataProvider == NULL) {
		return NULL;
	}
	context->myMedaData.myPersonCount = 0;
	context->myMedaData.myPersons = NULL;
	context->myDefaultString = "N/A";
	return context;
}

FT_API void
Free(Context* aContext) {
	if (aContext == NULL) {
		return;
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
}

FT_API Context*
CreateWithCSVAndJSON(const char* aPath) {
	Context* context = Create();
	if (context == NULL) {
		return context;
	}

	IPlatform* platform = CreateDefaultIo(context);
	IRelationals* relations = CreateCSVRelations(context, aPath, platform);
	IPersonals* personals = CreateJSONPersonals(context, aPath, platform);
	IDataProvider* provider = CreateDataProvider(context, relations, personals);
	AddDataProvider(context, provider);

	return context;
}

FT_API Context*
CreateCSVAndJSONWithIO(const char* aPath, IPlatform* aPlatform) {
	Context* context = Create();
	if (context == NULL) {
		return context;
	}

	IRelationals* relations = CreateCSVRelations(context, aPath, aPlatform);
	IPersonals* personals = CreateJSONPersonals(context, aPath, aPlatform);
	IDataProvider* provider = CreateDataProvider(context, relations, personals);
	AddDataProvider(context, provider);

	return context;
}

FT_API void
AddDataProvider(Context* aContext, IDataProvider* aData) {
	if (aContext == NULL) {
		return;
	}
	if (aData == NULL) {
		return;
	}
	if (aData->Copy == NULL) {
		return;
	}
	if (aData->GetAllIds == NULL) {
		return;
	}
	if (aData->GetAllIdsCount == NULL) {
		return;
	}
	if (aData->GetAllRelationsOfId == NULL) {
		return;
	}
	if (aData->GetAllRelationsOfIdCount == NULL) {
		return;
	}
	if (aData->GetPerson == NULL) {
		return;
	}
	if (aData->PlayPerson == NULL) {
		return;
	}

	ProviderComposit_AddDataProvider(aContext->myDataProvider, aData->Copy(aData));
	FreeMetaData(&aContext->myMedaData);
	aContext->myMedaData = CreateMetaData(ProviderComposit_Cast(aContext->myDataProvider));
}

FT_API IDataProvider*
CreateDataProvider(Context* aContext, IRelationals* aRelations, IPersonals* aPersonals) {
	if (aContext == NULL) {
		return NULL;
	}
	if (aRelations->Copy == NULL) {
		return NULL;
	}
	if (aRelations->Free == NULL) {
		return NULL;
	}
	if (aRelations->GetAllIds == NULL) {
		return NULL;
	}
	if (aRelations->GetAllIdsCount == NULL) {
		return NULL;
	}
	if (aRelations->GetAllRelationsOfId == NULL) {
		return NULL;
	}
	if (aRelations->GetAllRelationsOfIdCount == NULL) {
		return NULL;
	}
	if (aPersonals->Copy == NULL) {
		return NULL;
	}
	if (aPersonals->Free == NULL) {
		return NULL;
	}
	if (aPersonals->GetAllIds == NULL) {
		return NULL;
	}
	if (aPersonals->GetAllIdsCount == NULL) {
		return NULL;
	}
	if (aPersonals->GetPerson == NULL) {
		return NULL;
	}
	if (aPersonals->PlayPerson == NULL) {
		return NULL;
	}
	if (aPersonals->ShowImages == NULL) {
		return NULL;
	}

	IDataProvider* data = CreateSharedDataProviderDecorator(
		CreateForwardingProvider(aRelations->Copy(aRelations), aPersonals->Copy(aPersonals)));
	if (data == NULL) {
		return NULL;
	}

	IDataProvider** newArray = realloc(
		aContext->myCreatedDataProviders,
		sizeof(IRelationals*) * (aContext->myCreatedDataProvidersSize + 1));
	if (newArray == NULL) {
		data->Free(data);
		free(data);
		return NULL;
	}

	newArray[aContext->myCreatedDataProvidersSize] = data;
	aContext->myCreatedDataProvidersSize++;
	aContext->myCreatedDataProviders = newArray;

	return data;
}

FT_API int
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
CreateCSVRelations(Context* aContext, const char* aPath, IPlatform* aPlatform) {
	if (aContext == NULL) {
		return NULL;
	}
	if (!CheckIo(aPlatform)) {
		return NULL;
	}

	IRelationals* relations =
		CreateSharedRelationalsDecorator(CreateCSVRelation(aPath, aPlatform->Copy(aPlatform)));
	if (relations == NULL) {
		return NULL;
	}

	IRelationals** newArray = realloc(
		aContext->myCreatedRelations,
		sizeof(IRelationals*) * (aContext->myCreatedRelationsSize + 1));
	if (newArray == NULL) {
		relations->Free(relations);
		free(relations);
		return NULL;
	}

	newArray[aContext->myCreatedRelationsSize] = relations;
	aContext->myCreatedRelationsSize++;
	aContext->myCreatedRelations = newArray;
	return relations;
}

FT_API IPersonals*
CreateJSONPersonals(Context* aContext, const char* aPath, IPlatform* aPlatform) {
	if (aContext == NULL) {
		return NULL;
	}
	if (!CheckIo(aPlatform)) {
		return NULL;
	}

	IPersonals* persons =
		CreateSharedPersonalsDecorator(CreateJSONPerson(aPath, aPlatform->Copy(aPlatform)));
	if (persons == NULL) {
		return NULL;
	}

	IPersonals** newArray = realloc(
		aContext->myCreatedPersonals,
		sizeof(IRelationals*) * (aContext->myCreatedPersonalsSize + 1));
	if (newArray == NULL) {
		persons->Free(persons);
		free(persons);
		return NULL;
	}

	newArray[aContext->myCreatedPersonalsSize] = persons;
	aContext->myCreatedPersonalsSize++;
	aContext->myCreatedPersonals = newArray;
	return persons;
}

FT_API IPlatform*
CreateDefaultIo(Context* aContext) {
	if (aContext == NULL) {
		return NULL;
	}

	IPlatform* platform = CreateSharedPlatformDecorator(CreatePlatform());
	if (platform == NULL) {
		return NULL;
	}

	IPlatform** newArray = realloc(
		aContext->myCreatedPlatforms,
		sizeof(IRelationals*) * (aContext->myCreatedPlatformSize + 1));
	if (newArray == NULL) {
		platform->Free(platform);
		free(platform);
		return NULL;
	}

	newArray[aContext->myCreatedPlatformSize] = platform;
	aContext->myCreatedPlatformSize++;
	aContext->myCreatedPlatforms = newArray;
	return platform;
}

void
PopulateNullValues(Person* aPerson, const char* aDefault) {
	if (aPerson->title == NULL) {
		aPerson->title = aDefault;
	}
	if (aPerson->titleOfNobility == NULL) {
		aPerson->titleOfNobility = aDefault;
	}
	if (aPerson->gender == NULL) {
		aPerson->gender = aDefault;
	}
	if (aPerson->dateOfBirth == NULL) {
		aPerson->dateOfBirth = aDefault;
	}
	if (aPerson->placeOfBirth == NULL) {
		aPerson->placeOfBirth = aDefault;
	}
	if (aPerson->dateOfDeath == NULL) {
		aPerson->dateOfDeath = aDefault;
	}
	if (aPerson->placeOfDeath == NULL) {
		aPerson->placeOfDeath = aDefault;
	}
	if (aPerson->remark == NULL) {
		aPerson->remark = aDefault;
	}
}

FT_API Person
GetPerson(Context* aContext, PersonId aId) {
	Person p;
	p.id = 0;
	p.title = "";
	p.firstNameCount = 0;
	p.firstNames = NULL;
	p.titleOfNobility = "";
	p.lastNameCount = 0;
	p.lastNames = NULL;
	p.gender = "";
	p.dateOfBirth = "";
	p.placeOfBirth = "";
	p.dateOfDeath = "";
	p.placeOfDeath = "";
	p.remark = "";

	if (aContext == NULL) {
		return p;
	}

	IDataProvider* interface = ProviderComposit_Cast(aContext->myDataProvider);
	interface->GetPerson(interface, aId, &p);
	PopulateNullValues(&p, aContext->myDefaultString);
	return p;
}

FT_API void
PlayPerson(Context* aContext, PersonId aId) {
	if (aContext == NULL) {
		return;
	}

	IDataProvider* interface = ProviderComposit_Cast(aContext->myDataProvider);
	interface->PlayPerson(interface, aId);
}

FT_API void
ShowImagesOfPerson(Context* aContext, PersonId aId) {
	if (aContext == NULL) {
		return;
	}

	IDataProvider* interface = ProviderComposit_Cast(aContext->myDataProvider);
	interface->ShowImages(interface, aId);
}

FT_API Person*
GetPersonsMatchingPattern(
	Context* aContext, Person aPrototype, int aMinMatches, size_t* aOutPersonsCount) {
	if (aContext == NULL) {
		return NULL;
	}
	IDataProvider* data = ProviderComposit_Cast(aContext->myDataProvider);
	size_t idCount = data->GetAllIdsCount(data);
	PersonId* ids = malloc(idCount * sizeof(PersonId));
	data->GetAllIds(data, ids);

	Person* result = calloc(1, sizeof(Person));
	size_t count = 0;
	int found = 0;
	for (size_t i = 0; i < idCount; i++) {
		data->GetPerson(data, ids[i], result + count);

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
			PopulateNullValues(result + count, aContext->myDefaultString);
			count++;
			result = realloc(result, (count + 1) * sizeof(Person));
			continue;
		}
	}
	free(ids);

	free(aContext->myCreatedPersons);
	aContext->myCreatedPersons = result;

	*aOutPersonsCount = count;
	return result;
}

FT_API Relation*
GetPersonRelations(Context* aContext, PersonId aId, size_t* aOutRelationsCount) {
	if (aContext == NULL) {
		return NULL;
	}
	IDataProvider* data = ProviderComposit_Cast(aContext->myDataProvider);
	size_t count = data->GetAllRelationsOfIdCount(data, aId);
	Relation* result = malloc(sizeof(Relation) * count);
	data->GetAllRelationsOfId(data, aId, result);

	*aOutRelationsCount = count;
	if (result->relationship == NULL) {
		result->relationship = aContext->myDefaultString;
	}
	if (result->startDate == NULL) {
		result->startDate = aContext->myDefaultString;
	}
	if (result->endDate == NULL) {
		result->endDate = aContext->myDefaultString;
	}
	return result;
}

FT_API int
GetRelativeGeneration(Context* aContext, PersonId aRefId, PersonId aTargetId) {
	if (aContext == NULL) {
		return INT_MIN;
	}

	return ComputeRelativeGeneration(aContext->myMedaData, aRefId, aTargetId);
}

FT_API int
IsDefaultString(Context* aContext, const char* aStringToCheck) {
	if (aContext == NULL) {
		return 0;
	}
	return aContext->myDefaultString == aStringToCheck;
}
