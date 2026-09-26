#ifndef FT_API_H
#define FT_API_H

#include <stddef.h>

// NOLINTBEGIN
#ifdef __cplusplus
extern "C" {
#endif
/// @cond DO_NOT_DOCUMENT
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
/// @endcond

/// @defgroup Types Types
/// @{

/// @brief a opaque type used for almost all api calls.
/// @note the api is not threadsave.
typedef struct Context Context;

/// @brief an identifyer for a person. this may not match the id in the data.
typedef size_t PersonId;

typedef const char* Utf8_String;

/// @brief places of residence can also have a start- and enddate.
typedef struct {
	/// the name of the residence.
	Utf8_String name;
	/// the date, when the person moved here.
	Utf8_String startDate;
	/// the date the person moved away from here.
	Utf8_String endDate;
} Residence;

/// @brief all detailed informations about a person.
/// @note all Utf8_String fields will be valid and may be filled with a default string. same goes
/// for the first element in any Utf8_String const* array even if its Count variable is 0. it is
/// therefore always save to do the following:
/// @code{.c}
/// printf("%s %s. genre: %s", firstNames[0], lastNames[0], genre);
/// @endcode
/// a string can be checked if it is the default value by using @ref IsDefaultString
typedef struct {
	/// his id used in this context instance.
	PersonId id;
	/// title if given. e.g. "Dr."
	Utf8_String title;
	/// the size of the @a firstNames array.
	size_t firstNameCount;
	/// all known first names of the person.
	Utf8_String const* firstNames;
	/// a title of nobility if given. e.g. "van"
	Utf8_String titleOfNobility;
	/// the size of the @a lastNames array.
	size_t lastNameCount;
	/// all known last names of the person.
	Utf8_String const* lastNames;
	/// the gender of the person.
	Utf8_String gender;
	/// the birthdate in an unspecifyed format.
	Utf8_String dateOfBirth;
	/// the place of birth in an unspecifyed format.
	Utf8_String placeOfBirth;
	/// the date of death in an unspecifyed format.
	Utf8_String dateOfDeath;
	/// the place of deat in an unspecifyed format.
	Utf8_String placeOfDeath;
	/// the size of the @a professions array.
	size_t professionCount;
	/// all known professions of the person.
	Utf8_String const* professions;
	/// the size of the @a placeOfResidences array.
	size_t placeOfResidenceCount;
	/// all known places of residence of the person.
	Residence const* placeOfResidences;
	/// any additional information may be added here.
	Utf8_String remark;
} Person;

/// @brief a relation between two people
/// @note all Utf8_String fields will be valid and may be filled with a default string. use @ref
/// IsDefaultString to see if it was default filled.
typedef struct {
	/// a id as given in this context instance.
	PersonId id1;
	/// a id as given in this context instance.
	PersonId id2;
	/// a string denoting the relationship of @a id1 to @a id2. e.g. "stepfather"
	Utf8_String relationship;
	/// the start of the relationship in an unspecifyed format.
	Utf8_String startDate;
	/// the start of the relationship in an unspecifyed format.
	Utf8_String endDate;
} Relation;

typedef struct{
	Person person;
	Utf8_String audio;
	size_t imageCount;
	Utf8_String* images;
}PersonExt;

/// @brief this enum is used in algorythms to programaticaly understand the nation of a
/// relationship.
typedef enum {
	/// this relation has no generation restriction e.g. a marriage.
	RelationType_Unrestricted = 0,
	/// id2 is in a lower generation then id1 e.g. a child.
	RelationType_StrictlyLower,
	/// id2 is in a higher generation then id1 e.g. a parent.
	RelationType_StrictlyHigher,
} RelationType;

typedef struct{
	Relation relation;
	/// the type of relation from @a id1 to @a id2.
	RelationType type;
}RelationExt;

typedef enum {
	QuestionType_ContextIsReady = 0,
	QuestionType_ListOfFolders = 1,
	QuestionType_ListOfFolders_UseDefault = -1,
	QuestionType_ContentOfFile = 2,
	QuestionType_ContentOfFile_UseDefault = -2,
	QuestionType_ParseStringToPerson = 3,
	QuestionType_ParseStringToPerson_UseDefault = -3,
	QuestionType_ParseStringToRelations = 4,
	QuestionType_ParseStringToRelations_UseDefault = -4,
} QuestionType;

typedef struct {
	size_t arrayCount;
	QuestionType question;
	union {
		Utf8_String qFilename;
		Utf8_String qFoldername;
		Utf8_String qString;
		Context* context;
	};
	union {
		PersonExt* aPerson;
		RelationExt* aRelations;
		Utf8_String aString;
		Utf8_String* aStrings;
	};
	void* data;
} ContextBuilder;

typedef struct {
} DebuggingData;

/// @cond
typedef struct GraphBuilderStrategy GraphBuilderStrategy;
/// @endcond

struct GraphBuilderStrategy {
	void (*PersonInGenerationStrategy)(GraphBuilderStrategy* aThis, PersonId aId, int aGeneration);
	/// the arrays may only be valid turing the execution of this function
	void (*FamilieStrategy)(
		GraphBuilderStrategy* aThis,
		PersonId* aParents,
		size_t aParentCount,
		PersonId* aChilds,
		size_t aChildCount);
};

/// @}

/// @defgroup API API
/// @{

/// @brief creates a context instance.
/// @param aDebug a trace object that reseaves all relevant internals. set it to @a NULL if not
/// interested.
/// @return a valid context.
FT_API Context* Create(Utf8_String aPath, DebuggingData* aDebug);
FT_API ContextBuilder StartCreate(Utf8_String aPath, DebuggingData* aDebug);
FT_API ContextBuilder Continue(ContextBuilder aPreviousStep, DebuggingData* aDebug);
FT_API Context* Merge(Context* aContext1, Context* aContext2, DebuggingData* aDebug);

/// @brief frees the context and all allocations done with that conetxt.
/// @param aContext the context to free.
/// @param aDebug a trace object that reseaves all relevant internals. set it to @a NULL if not
/// interested.
/// @note also frees all strings and classes returned in other functions suche as @ref
/// CreateCSVRelations or @ref GetPerson
FT_API void Free(Context* aContext, DebuggingData* aDebug);

/// @brief retreaves the Person information
/// @param aContext a pointer to the context.
/// @param aId the id of the person interested in.
/// @param aDebug a trace object that reseaves all relevant internals. set it to @a NULL if not
/// interested.
/// @return a filled struct with all details about the person requested in @a aId.
/// all const char* fields in the struct are guaranteed to be set.
/// the const char* const* fields have a guaranteed first element.
/// use @ref IsDefaultString to check if a field was filed with the default string.
/// @note the values in Person will be freed once @ref Free is called.
FT_API Person GetPerson(Context* aContext, PersonId aId, DebuggingData* aDebug);

/// @brief causes the Platform to play the audio if any such file is given in the data and exists.
/// @param aContext a pointer to the context.
/// @param aId the id of the person interested in.
/// @param aDebug a trace object that reseaves all relevant internals. set it to @a NULL if not
/// interested.
FT_API void PlayPerson(Context* aContext, PersonId aId, DebuggingData* aDebug);

/// @brief causes the Platform to show all image files if any such file is given in the data and
/// exists.
/// @param aContext a pointer to the context.
/// @param aId the id of the person interested in.
/// @param aDebug a trace object that reseaves all relevant internals. set it to @a NULL if not
/// interested.
FT_API void ShowImagesOfPerson(Context* aContext, PersonId aId, DebuggingData* aDebug);

/// @brief searches the data for matching people
/// @param aContext a pointer to the context.
/// @param aPrototype a Person object that is used to match people against.
/// @note all values that are NULL are ignored in the search.
/// @param aMinMatches the amount of fields that have to match at least to the @a aPrototype to be
/// considered a match.
/// @param aOutPersonsCount the size of the array returned.
/// @param aDebug a trace object that reseaves all relevant internals. set it to @a NULL if not
/// interested.
/// @return a pointer to an array of @ref Person which match the given @a aPrototype.
/// @note the object will be freed at the next call or once @ref Free is called.
FT_API Person* GetPersonsMatchingPattern(
	Context* aContext,
	Person aPrototype,
	size_t aMinMatches,
	size_t* aOutPersonsCount,
	DebuggingData* aDebug);

/// @brief retreaves all relations of a person.
/// @param aContext a pointer to the context.
/// @param aId the id of the person interested in.
/// @param aOutRelationsCount the size of the array returned.
/// @param aDebug a trace object that reseaves all relevant internals. set it to @a NULL if not
/// interested.
/// @return a pointer to an array of @ref Relation which contain @a aId.
/// @note the values in Person will be freed once @ref Free is called.
/// @todo this array is currently leaked.
FT_API Relation*
GetPersonRelations(Context* aContext, PersonId aId, size_t* aOutRelationsCount, DebuggingData* aDebug);

/// @brief retreaves the relative generation of @a aTargetId.
/// @param aContext a pointer to the context.
/// @param aRefId the id which is considered generation 0.
/// @param aTargetId the id of the person interested in.
/// @param aDebug a trace object that reseaves all relevant internals. set it to @a NULL if not
/// interested.
/// @return the number of generations @a aTargetId is apart from @a aRefId. a positive number means
/// lower in the graph. a negative number means higher in the graph. 0 means in the same generation.
FT_API int
GetRelativeGeneration(Context* aContext, PersonId aRefId, PersonId aTargetId, DebuggingData* aDebug);

/// @brief retreaves all people @a aId has at least one common child with.
/// @param aContext a pointer to the context.
/// @param aId the id of the person interested in.
/// @param aOutParterCount the amount of partners returned.
/// @param aDebug a trace object that reseaves all relevant internals. set it to @a NULL if not
/// interested.
/// @return an array of Id's or NULL if no partner is found or @a aId has no children.
FT_API PersonId*
GetPartners(Context* aContext, PersonId aId, size_t* aOutParterCount, DebuggingData* aDebug);

/// @brief retreaves all people @a aId has at least one common parent with.
/// @param aContext a pointer to the context.
/// @param aId the id of the person interested in.
/// @param aOutSiblingCount the amount of siblings returned.
/// @param aDebug a trace object that reseaves all relevant internals. set it to @a NULL if not
/// interested.
/// @return an array of Id's or NULL if no sibling is found or @a aId has no parent.
FT_API PersonId*
GetSiblings(Context* aContext, PersonId aId, size_t* aOutSiblingCount, DebuggingData* aDebug);

/// @brief retreaves the common parents between all people provided in @a aIds.
/// @param aContext a pointer to the context.
/// @param aIdCount the size of @a aIds.
/// @param aIds the id's of the childrens the parents are searched for. the array must have @a
/// aIdCount amount of elements allocated.
/// @param aOutParentCount the amount of parents returned.
/// @param aDebug a trace object that reseaves all relevant internals. set it to @a NULL if not
/// interested.
/// @return an array of Id's that are marked as parent in all @a aIds or NULL if there are no common
/// Parents.
FT_API PersonId* GetCommonParents(
	Context* aContext,
	size_t aIdCount,
	PersonId* aIds,
	size_t* aOutParentCount,
	DebuggingData* aDebug);

/// @brief retreaves the common children between all people provided in @a aIds.
/// @param aContext a pointer to the context.
/// @param aIdCount the size of @a aIds.
/// @param aIds the id's of the parents the childrens are searched for. the array must have @a
/// aIdCount amount of elements allocated.
/// @param aOutChildrenCount the amount of children returned.
/// @param aDebug a trace object that reseaves all relevant internals. set it to @a NULL if not
/// interested.
/// @return an array of Id's that are marked as children in all @a aIds or NULL if there are no
/// common Children.
FT_API PersonId* GetCommonChildren(
	Context* aContext,
	size_t aIdCount,
	PersonId* aIds,
	size_t* aOutChildrenCount,
	DebuggingData* aDebug);

/// @brief a shortcut method using a bunch of the previous apis to build up a graph.
/// @param aContext a pointer to the context.
/// @param aId the id of the person interested in.
/// @param aDistance the amount of relational hops from @a aId away still included in the graph.
/// @param aStrategy a collection of function to whome calculated data is returned to.
/// @param aDebug a trace object that reseaves all relevant internals. set it to @a NULL if not
/// interested.
FT_API void BuildGraph(
	Context* aContext,
	PersonId aId,
	size_t aDistance,
	GraphBuilderStrategy* aStrategy,
	DebuggingData* aDebug);

/// @brief checks if a given string points to the default value.
/// @param aContext a pointer to the context.
/// @param aStringToCheck the string to check.
/// @return 1 if @a aStringToCheck points at the adress the default string is stored in. otherwise
/// 0.
FT_API int IsDefaultString(Context* aContext, const char* aStringToCheck);

/// @}

#ifdef __cplusplus
}
#endif
// NOLINTEND
#endif // FT_API_H
