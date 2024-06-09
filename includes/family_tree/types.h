#ifndef FT_TYPES_H
#define FT_TYPES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	RelationType_Unrestricted = 0,
	RelationType_StrictlyLower,
	RelationType_StrictlyHigher,
} RelationType;

typedef struct Context Context;
typedef int PersonId;

typedef struct {
	PersonId id;
	const char* title;
	size_t firstNameCount;
	const char* const* firstNames;
	const char* titleOfNobility;
	size_t lastNameCount;
	const char* const* lastNames;
	const char* gender;
	const char* dateOfBirth;
	const char* placeOfBirth;
	const char* dateOfDeath;
	const char* placeOfDeath;
	const char* remark;
} Person;

typedef struct {
	PersonId id1;
	PersonId id2;
	const char* relationship;
	const char* startDate;
	const char* endDate;
} Relation;

typedef struct IRelationals IRelationals;
struct IRelationals {
	IRelationals* (*Copy)(IRelationals* aThis);
	size_t (*GetAllIdsCount)(IRelationals* aThis);
	void (*GetAllIds)(IRelationals* aThis, PersonId* aOutId);
	size_t (*GetAllRelationsOfIdCount)(IRelationals* aThis, PersonId aId);
	void (*GetAllRelationsOfId)(IRelationals* aThis, PersonId aId, Relation* aOutRelation);
	RelationType (*GetRelationType)(IRelationals* aThis, Relation aRelation);
	void (*Free)(IRelationals* aThis);
};

typedef struct IPersonals IPersonals;
struct IPersonals {
	IPersonals* (*Copy)(IPersonals* aThis);
	size_t (*GetAllIdsCount)(IPersonals* aThis);
	void (*GetAllIds)(IPersonals* aThis, PersonId* aOutId);
	void (*GetPerson)(IPersonals* aThis, PersonId aId, Person* aOutPerson);
	void (*PlayPerson)(IPersonals* aThis, PersonId aId);
	void (*ShowImages)(IPersonals* aThis, PersonId aId);
	void (*Free)(IPersonals* aThis);
};

typedef struct IDataProvider IDataProvider;
struct IDataProvider {
	IDataProvider* (*Copy)(IDataProvider* aThis);
	void (*GetPerson)(IDataProvider* aThis, PersonId aId, Person* aOutPerson);
	void (*PlayPerson)(IDataProvider* aThis, PersonId aId);
	void (*ShowImages)(IDataProvider* aThis, PersonId aId);
	size_t (*GetAllIdsCount)(IDataProvider* aThis);
	void (*GetAllIds)(IDataProvider* aThis, PersonId* aOutId);
	size_t (*GetAllRelationsOfIdCount)(IDataProvider* aThis, PersonId aId);
	void (*GetAllRelationsOfId)(IDataProvider* aThis, PersonId aId, Relation* aOutRelation);
	RelationType (*GetRelationType)(IDataProvider* aThis, Relation aRelation);
	void (*Free)(IDataProvider* aThis);
};

typedef struct IPlatform IPlatform;
struct IPlatform {
	IPlatform* (*Copy)(IPlatform* aThis);
	// NOTE: returns foldernames in seperated with nullterminators and ends the list with two
	// nullterminators. e.g. "FolderA\0FolderB\0FolderC\0\0";
	char* (*GetFolders)(IPlatform* aThis, const char* aPath);
	char* (*ReadFile)(IPlatform* aThis, const char* aPath);
	void (*FreeString)(IPlatform* aThis, char* aString);
	void (*OpenAudio)(IPlatform* aThis, const char* aPath);
	void (*OpenImage)(IPlatform* aThis, const char* aPath);
	void (*Free)(IPlatform* aThis);
};

#ifdef __cplusplus
}
#endif
#endif // FT_TYPES_H