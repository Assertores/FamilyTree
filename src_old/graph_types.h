#ifndef GRAPH_TYPES_H
#define GRAPH_TYPES_H
#include "application_types.h"
#include "base_types.h"
#include "generic_types.h"

typedef struct PersonMeta_t PersonMeta;
SetRef(PersonMeta);

typedef struct PersonMeta_t {
	Person* person;
	PersonMetas parents;
	PersonMetas childs;

	int myGeneration;
	Bool isProxy;
	int briefWidth;
} PersonMeta;
Bool EqualPersonMeta(const PersonMeta* aLhs, const PersonMeta* aRhs);
void FreePersonMeta(PersonMeta aPersonMeta);

ExpectedRef(PersonMeta);
typedef PersonMeta ArrPersonMeta;
Array(ArrPersonMeta);
typedef PersonMeta ArrRefPersonMeta;
ArrayRef(ArrRefPersonMeta);

typedef ArrRefPersonMetas Generation;
Array(Generation);

typedef struct {
	PersonMetas parents;
	PersonMetas childs;
} Family;
Bool EqualFamily(const Family* aLhs, const Family* aRhs);
void FreeFamily(Family aFamily);

Set(Family);
ExpectedRef(Family);

EPersonMeta FindTreeElement(const ArrPersonMetas* aData, int aPerson);
EPersonMeta FindPersonMeta(const Generation* aData, int aPerson);
#endif // GRAPH_TYPES_H