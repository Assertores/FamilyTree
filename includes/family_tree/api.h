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

//! @brief creates a context instance.
//! @return a valid context.
FT_API Context* Create();

//! @brief frees the context and all allocations done with that conetxt.
//! @param aContext the context to free.
//! @note also frees all strings and classes returned in other functions suche as @ref
//! CreateCSVRelations or @ref GetPerson
FT_API void Free(Context* aContext);

//! @brief creates a context instance.
//! @param aPath the path to read in. this will be used for @ref CreateCSVRelations and @ref
//! CreateJSONPersonals.
//! @return a fully setup context with csv relations and json persons already read in using the
//! default Platform.
FT_API Context* CreateWithCSVAndJSON(const char* aPath);

//! @brief creates a context instance.
//! @param aPath the path to read in. this will be used for @ref CreateCSVRelations and @ref
//! CreateJSONPersonals.
//! @param aPlatform the platform used by @ref CreateCSVRelations and @ref CreateJSONPersonals.
//! @return a fully setup context with csv relations and json persons already read in.
FT_API Context* CreateCSVAndJSONWithIO(const char* aPath, IPlatform* aPlatform);

//! @brief adds a data provider to the context.
//! @param aContext a pointer to the context.
//! @param aDataProvider the data provider to add. @see AddDataProvider.
//! @note this will also recalculate the graph.
FT_API void AddDataProvider(Context* aContext, IDataProvider* aDataProvider);

//! @brief creates a data provider that can be used in @ref AddDataProvider
//! @param aContext a pointer to the context.
//! @param aRelations the relations to which the data provider shall forward.
//! @param aPersonals the personal data to which the data provider shall forward.
//! @return a refcounted forwarding data provider.
//! @note Copy will be called on @a aRelations and @a aPersonals.
//! @note the object will be freed once @ref Free is called.
FT_API IDataProvider* CreateDataProvider(
	Context* aContext, IRelationals* aRelations, IPersonals* aPersonals);

//! @brief creates a relations object used in @ref CreateDataProvider
//! @param aContext a pointer to the context.
//! @param aPath the path to a folder containing a "relations.csv" file that contains relational
//! data in the following structure:
//! @code{.csv}
//! id1,id2,type,relatonship,start,end
//! @endcode
//! @note the first line is assumed to be the header and is ignored.
//! @note only the first 3 elements a mandatory.
//!   - @a id1 needs to be a number (refering to a person).
//!   - @a id2 needs to be a number (refering to a person).
//!   - @a type needs be one of 3 values:
//!     - @a Unrestricted this relation has no generation restriction e.g. a marriage.
//!     - @a StrictlyLower id2 is in a lower generation then id1 e.g. a child.
//!     - @a StrictlyHigher id2 is in a higher generation then id1 e.g. a parent.
//! all other fields may contain arbitrary string values.
//! @param aPlatform this is used to read in the csv file.
//! @return a fully setup object that can be used to retreave relation data information.
//! @note the object will be freed once @ref Free is called.
FT_API IRelationals* CreateCSVRelations(Context* aContext, const char* aPath, IPlatform* aPlatform);

//! @brief creates a personals object used in @ref CreateDataProvider
//! @param aContext a pointer to the context.
//! @param aPath the path to a folder which contains subfolders containing data.json files in the
//! following format:
//! @code{.json}
//! {
//! "person": 1,
//! "title": "Some Title",
//! "firstNames": [ "Name1", "Name2" ],
//! "titleOfNobility": "Another Title",
//! "lastName": [ "Name1", "Name2" ],
//! "gender": "my Genre",
//! "dateOfBirth": "a fitting date",
//! "placeOfBirth": "a fitting place",
//! "death": "hopefully a different date",
//! "placeOfDeath": "some place somewhere",
//! "Audio": "a path to a audiofile relative to this file",
//! "Images": [ "a path to a image relative to this file", "another image" ],
//! "Remarks": "some extra information about the person"
//! }
//! @endcode
//! @note the files content needs to be a valid json format. it must contain the "person" key which
//! must be a integer value. all other fields are optional and can be omitted.
//! @param aPlatform
//! @return a fully setup object that can be used to retreave personal data.
//! @note the object will be freed once @ref Free is called.
FT_API IPersonals* CreateJSONPersonals(Context* aContext, const char* aPath, IPlatform* aPlatform);

//! @brief Create a Platform used in @ref CreateCSVRelations and @ref CreateJSONPersonals
//! @param aContext a pointer to the context.
//! @return a windows implimentation if compiled for windows, a linux implimentation for linux,
//! otherwise NULL.
//! @todo impliment linux
//! @note the object will be freed once @ref Free is called.
FT_API IPlatform* CreateDefaultPlatform(Context* aContext);

//! @brief retreaves the Person information
//! @param aContext a pointer to the context.
//! @param aId the id of the person interested in.
//! @return a filled struct with all details about the person requested in @a aId.
//! all const char* fields in the struct are guaranteed to be set.
//! the const char* const* fields have a guaranteed first element.
//! use @ref IsDefaultString to check if a field was filed with the default string.
//! @note the values in Person will be freed once @ref Free is called.
FT_API Person GetPerson(Context* aContext, PersonId aId);

//! @brief causes the Platform to play the audio if any such file is given in the data and exists.
//! @param aContext a pointer to the context.
//! @param aId the id of the person interested in.
FT_API void PlayPerson(Context* aContext, PersonId aId);

//! @brief causes the Platform to show all image files if any such file is given in the data and
//! exists.
//! @param aContext a pointer to the context.
//! @param aId the id of the person interested in.
FT_API void ShowImagesOfPerson(Context* aContext, PersonId aId);

//! @brief searches the data for matching people
//! @param aContext a pointer to the context.
//! @param aPrototype a Person object that is used to match people against.
//! @note all values that are NULL are ignored in the search.
//! @param aMinMatches the amount of fields that have to match at least to the @a aPrototype to be
//! considered a match.
//! @param aOutPersonsCount the size of the array returned.
//! @return a pointer to an array of @ref Person which match the given @a aPrototype.
//! @note the object will be freed at the next call or once @ref Free is called.
FT_API Person* GetPersonsMatchingPattern(
	Context* aContext, Person aPrototype, size_t aMinMatches, size_t* aOutPersonsCount);

//! @brief retreaves all relations of a person.
//! @param aContext a pointer to the context.
//! @param aId the id of the person interested in.
//! @param aOutRelationsCount the size of the array returned.
//! @return a pointer to an array of @ref Relation which contain @a aId.
//! @note the values in Person will be freed once @ref Free is called.
//! @todo this array is currently leaked.
FT_API Relation* GetPersonRelations(Context* aContext, PersonId aId, size_t* aOutRelationsCount);

//! @brief retreaves the relative generation of @aTargetId
//! @param aContext a pointer to the context.
//! @param aRefId the id which is considered generation 0.
//! @param aTargetId the id of the person interested in.
//! @return the number of generations @a aTargetId is apart from @a aRefId. a positive number means
//! lower in the graph. a negative number means higher in the graph. 0 means in the same generation.
FT_API int GetRelativeGeneration(Context* aContext, PersonId aRefId, PersonId aTargetId);

//! @brief checks if a given string points to the default value.
//! @param aContext a pointer to the context.
//! @param aStringToCheck the string to check.
//! @return 1 if @a aStringToCheck points at the adress the default string is stored in. otherwise
//! 0.
FT_API int IsDefaultString(Context* aContext, const char* aStringToCheck);

#ifdef __cplusplus
}
#endif
#endif // FT_API_H