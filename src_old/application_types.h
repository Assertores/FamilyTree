#ifndef APPLICATION_TYPES_H
#define APPLICATION_TYPES_H
#include "base_types.h"
#include "generic_types.h"

Array(String);

typedef struct {
	int person1;
	int person2;
} Relation;
Bool EqualRelation(const Relation* aLhs, const Relation* aRhs);

Set(Relation);
Expected(Relation);
Expected(Relations);

typedef struct {
	int person;
	String title;
	Strings firstNames;
	String titleOfNobility;
	Strings lastNames;
	String gender;
	String birthDay;
	String birthPlace;
	String deathDay;
	String deathPlace;
	String remark;

	Path audioPath;
	Paths imagePaths;
} Person;
Bool EqualPerson(const Person* aLhs, const Person* aRhs);
/// default constructed fields are considered NOT set.
void MergePersonalData(Person* aPerson, Person aData, Bool aPrefereData);
void FreePerson(Person aPerson);

Set(Person);
Expected(Person);
Expected(Persons);
typedef Person ArrPerson;
ArrayRef(ArrPerson);

typedef struct {
	Persons persons;
	Relations relations;
} Graph;
void FreeGraph(Graph aGraph);

#endif // APPLICATION_TYPES_H
