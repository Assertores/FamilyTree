#ifndef FT_API_H
#define FT_API_H
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#ifndef FT_API
#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#define IMPORT __declspec(dllimport)
#elif __GNUC__
#define EXPORT __attribute__((visibility("default")))
#define IMPORT
#else // _MSC_VER
#error platform not suppored
#endif // _MSC_VER

#if FT_BUILD_STATIC
#define FT_API
#elif FT_BUILD_LIBRARY
#define FT_API EXPORT
#else // FT_BUILD_STATIC
#define FT_API IMPORT
#endif // FT_BUILD_STATIC
#endif // FT_API

FT_API Context* Create();
FT_API void Free(Context* aContext);

FT_API Context* CreateWithCSVAndJSON(const char* aPath);
FT_API Context* CreateCSVAndJSONWithIO(const char* aPath, IPlatform* aPlatform);

FT_API void AddDataProvider(Context* aContext, IDataProvider* aDataProvider);

// NOTE: FreeContext will free all here created structs
FT_API IDataProvider* CreateDataProvider(
	Context* aContext, IRelationals* aRelations, IPersonals* aPersonals);
FT_API IRelationals* CreateCSVRelations(Context* aContext, const char* aPath, IPlatform* aPlatform);
FT_API IPersonals* CreateJSONPersonals(Context* aContext, const char* aPath, IPlatform* aPlatform);
FT_API IPlatform* CreateDefaultIo(Context* aContext);

FT_API Person GetPerson(Context* aContext, PersonId aId);
FT_API void PlayPerson(Context* aContext, PersonId aId);
FT_API void ShowImagesOfPerson(Context* aContext, PersonId aId);

// NOTE: ignores default constructed elements in aPrototype. if aMinMatches is 0 then all non
// default elements need to match.
FT_API Person* GetPersonsMatchingPattern(
	Context* aContext, Person aPrototype, int aMinMatches, size_t* aOutPersonsCount);
FT_API Relation* GetPersonRelations(Context* aContext, PersonId aId, size_t* aOutRelationsCount);
FT_API int GetRelativeGeneration(Context* aContext, PersonId aRefId, PersonId aTargetId);
FT_API int IsDefaultString(Context* aContext, const char* aStringToCheck);

#ifdef __cplusplus
}
#endif
#endif // FT_API_H