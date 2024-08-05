#ifndef FT_TYPES_H
#define FT_TYPES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

//! @brief a opaque type used for almost all api calls.
//! @note the api is not threadsave.
typedef struct Context Context;

//! @brief an identifyer for a person. this may not match the id in the data.
typedef size_t PersonId;

//! @brief all detailed informations about a person.
//! @note all const char* fields will be valid and may be filled with a default string. same goes
//! for the first element in any const char* const* array even if its Count variable is 0. it is
//! therefore always save to do the following:
//! @code{.c}
//! printf("%s %s. genre: %s", firstNames[0], lastNames[0], genre);
//! @endcode
//! a string can be checked if it is the default value by using @ref IsDefaultString
typedef struct {
	//! his id used in this context instance.
	PersonId id;
	//! title if given. e.g. "Dr."
	const char* title;
	//! the size of the @a firstNames array.
	size_t firstNameCount;
	//! all known first names of the person.
	const char* const* firstNames;
	//! a title of nobility if given. e.g. "van"
	const char* titleOfNobility;
	//! the size of the @a lastNames array.
	size_t lastNameCount;
	//! all known last names of the person.
	const char* const* lastNames;
	//! the gender of the person.
	const char* gender;
	//! the birthdate in an unspecifyed format.
	const char* dateOfBirth;
	//! the place of birth in an unspecifyed format.
	const char* placeOfBirth;
	//! the date of death in an unspecifyed format.
	const char* dateOfDeath;
	//! the place of deat in an unspecifyed format.
	const char* placeOfDeath;
	//! any additional information may be added here.
	const char* remark;
} Person;

//! @brief a relation between two people
//! @note all const char* fields will be valid and may be filled with a default string. use @ref
//! IsDefaultString to see if it was default filled.
typedef struct {
	//! a id as given in this context instance.
	PersonId id1;
	//! a id as given in this context instance.
	PersonId id2;
	//! a string denoting the relationship of @a id1 to @a id2. e.g. "stepfather"
	const char* relationship;
	//! the start of the relationship in an unspecifyed format.
	const char* startDate;
	//! the start of the relationship in an unspecifyed format.
	const char* endDate;
} Relation;

//! @brief hints of common sets of people. all parents have these childrens and all childrens stem
//! from the parents.
typedef struct {
	//! the size of the @a parents array.
	size_t parentCount;
	//! all common parents of the id's in @a childrens.
	PersonId* parents;
	//! the size of the @a childrens array.
	size_t childrenCount;
	//! all common children of the id's in @a parents.
	PersonId* childrens;
} Family;

//! @brief this enum is used in algorythms to programaticaly understand the nation of a
//! relationship.
typedef enum {
	//! this relation has no generation restriction e.g. a marriage.
	RelationType_Unrestricted = 0,
	//! id2 is in a lower generation then id1 e.g. a child.
	RelationType_StrictlyLower,
	//! id2 is in a higher generation then id1 e.g. a parent.
	RelationType_StrictlyHigher,
} RelationType;

//! @cond
typedef struct IRelationals IRelationals;
typedef struct IPersonals IPersonals;
typedef struct IDataProvider IDataProvider;
typedef struct IPlatform IPlatform;
typedef struct ITrace ITrace;
//! @endcond

//! @brief an interface through which relational data can be retreaved.
struct IRelationals {
	//! returns a copy of the object.
	IRelationals* (*Copy)(IRelationals* aThis, ITrace* aTrace);
	//! returns the amount of ellements that need to be able to fit in @a aOutId provided to @a
	//! GetAllIds.
	size_t (*GetAllIdsCount)(IRelationals* aThis, ITrace* aTrace);
	//! stores all known id's into @a aOutId call @a GetAllIdsCount upfront and make shure enough
	//! memory is allocated.
	void (*GetAllIds)(IRelationals* aThis, PersonId* aOutId, ITrace* aTrace);
	//! returns the amount of ellements that need to be able to fit in @a aOutRelation provided to
	//! @a GetAllRelationsOfId.
	size_t (*GetAllRelationsOfIdCount)(IRelationals* aThis, PersonId aId, ITrace* aTrace);
	//! stores all known relations that contain @a aId into @a aOutRelation call @a
	//! GetAllRelationsOfIdCount upfront and make shure enough memory is allocated.
	void (*GetAllRelationsOfId)(
		IRelationals* aThis, PersonId aId, Relation* aOutRelation, ITrace* aTrace);
	//! returns the corresponding @ref RelationType for a given @a Relation
	RelationType (*GetRelationType)(IRelationals* aThis, Relation aRelation, ITrace* aTrace);
	//! frees the object itself and all data allocated by it.
	void (*Free)(IRelationals* aThis, ITrace* aTrace);
};

//! @brief an interface through which personal data can be retreaved.
struct IPersonals {
	//! returns a copy of the object.
	IPersonals* (*Copy)(IPersonals* aThis, ITrace* aTrace);
	//! returns the amount of ellements that need to be able to fit in @a aOutId provided to @a
	//! GetAllIds.
	size_t (*GetAllIdsCount)(IPersonals* aThis, ITrace* aTrace);
	//! stores all known id's into @a aOutId call @a GetAllIdsCount upfront and make shure enough
	//! memory is allocated.
	void (*GetAllIds)(IPersonals* aThis, PersonId* aOutId, ITrace* aTrace);
	//! retreave details about a given @a aId
	Person (*GetPerson)(IPersonals* aThis, PersonId aId, ITrace* aTrace);
	//! playes the audio of a given @a aId
	void (*PlayPerson)(IPersonals* aThis, PersonId aId, ITrace* aTrace);
	//! shows all images of a given @a aId
	void (*ShowImages)(IPersonals* aThis, PersonId aId, ITrace* aTrace);
	//! frees the object itself and all data allocated by it.
	void (*Free)(IPersonals* aThis, ITrace* aTrace);
};

//! @brief an interface combining @ref IRelationals and @ref IPersonals for conveniance.
struct IDataProvider {
	//! returns a copy of the object.
	IDataProvider* (*Copy)(IDataProvider* aThis, ITrace* aTrace);
	//! @copybrief IPersonals.GetPerson
	Person (*GetPerson)(IDataProvider* aThis, PersonId aId, ITrace* aTrace);
	//! @copybrief IPersonals.PlayPerson
	void (*PlayPerson)(IDataProvider* aThis, PersonId aId, ITrace* aTrace);
	//! @copybrief IPersonals.ShowImages
	void (*ShowImages)(IDataProvider* aThis, PersonId aId, ITrace* aTrace);
	//! @copybrief IPersonals.GetAllIdsCount
	size_t (*GetAllIdsCount)(IDataProvider* aThis, ITrace* aTrace);
	//! @copybrief IPersonals.GetAllIds
	void (*GetAllIds)(IDataProvider* aThis, PersonId* aOutId, ITrace* aTrace);
	//! @copybrief IRelationals.GetAllRelationsOfIdCount
	size_t (*GetAllRelationsOfIdCount)(IDataProvider* aThis, PersonId aId, ITrace* aTrace);
	//! @copybrief IRelationals.GetAllRelationsOfId
	void (*GetAllRelationsOfId)(
		IDataProvider* aThis, PersonId aId, Relation* aOutRelation, ITrace* aTrace);
	//! @copybrief IRelationals.GetRelationType
	RelationType (*GetRelationType)(IDataProvider* aThis, Relation aRelation, ITrace* aTrace);
	//! frees the object itself and all data allocated by it.
	void (*Free)(IDataProvider* aThis, ITrace* aTrace);
};

//! @brief an interface through which all comunication to the operating system is done.
struct IPlatform {
	//! returns a copy of the object.
	IPlatform* (*Copy)(IPlatform* aThis, ITrace* aTrace);
	//! returns subfoldernames seperated with nullterminators and ends the list with two
	//! nullterminators. e.g. "FolderA\0FolderB\0FolderC\0\0". free the string with @ref
	//! IPlatform.FreeString
	char* (*GetFolders)(IPlatform* aThis, const char* aPath, ITrace* aTrace);
	//! returns the content of a file given by @a aPath. free the string with @ref
	//! IPlatform.FreeString
	char* (*ReadFile)(IPlatform* aThis, const char* aPath, ITrace* aTrace);
	//! frees a string that was befor allocated by @ref IPlatform.GetFolders or @ref
	//! IPlatform.ReadFile.
	void (*FreeString)(IPlatform* aThis, char* aString, ITrace* aTrace);
	//! plays back the audiofile pointed at by @a aPath
	void (*OpenAudio)(IPlatform* aThis, const char* aPath, ITrace* aTrace);
	//! shows the image pointed at by @a aPath
	void (*OpenImage)(IPlatform* aThis, const char* aPath, ITrace* aTrace);
	//! frees the object itself and all data allocated by it.
	void (*Free)(IPlatform* aThis, ITrace* aTrace);
};

//! @brief an interface that will reseave all relevant info about the flow of the code.
//! @note this is ment for debuging purpises.
struct ITrace {
	//! creates a subtrace that is caused by @a aThis
	ITrace* (*CreateSubTrace)(ITrace* aThis, const char* aSubtraceName);
	//! adds an event to @a aThis trace
	void (*AddEvent)(ITrace* aThis, const char* aEvent);
	//! Succeeds and frees the trace.
	//!
	//! This indecates a premature success. don't call this if the floow succeeded normaly.
	//! @note don't call @ref ITrace.Free after calling @ref ITrace.Succeed
	void (*Succeed)(ITrace* aThis);
	//! Fails and frees the trace with a reason why it failed.
	//! @note don't call @ref ITrace.Free after calling @ref ITrace.Fail
	void (*Fail)(ITrace* aThis, const char* aReason);
	//! frees the object itself and all data allocated by it.
	void (*Free)(ITrace* aThis);
};

#ifdef __cplusplus
}
#endif
#endif // FT_TYPES_H
