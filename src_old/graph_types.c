#include "graph_types.h"

SetRefImpl(PersonMeta, EqualPersonMeta);
ArrayImpl(Generation, FreeArrRefPersonMetas);
SetImpl(Family, EqualFamily, FreeFamily);
ArrayImpl(ArrPersonMeta, FreePersonMeta);
ArrayRefImpl(ArrRefPersonMeta);

Bool
EqualPersonMeta(const PersonMeta* aLhs, const PersonMeta* aRhs) {
	return EqualPerson(aLhs->person, aRhs->person);
}

Bool
EqualFamily(const Family* aLhs, const Family* aRhs) {
	if (aLhs->parents.length != aRhs->parents.length) {
		return Different;
	}
	for (size_t i = 0; i < aLhs->parents.length; i++) {
		Bool result = EqualPersonMeta(aRhs->parents.data[i], aLhs->parents.data[i]);
		if (result != True) {
			return result;
		}
	}
	return True;
}

EPersonMeta
FindTreeElement(const ArrPersonMetas* aData, int aPerson) {
	for (size_t i = 0; i < aData->length; i++) {
		if (aData->data[i].person->person == aPerson) {
			EPersonMeta result = {0};
			result.data = &aData->data[i];
			result.hasData = True;
			return result;
		}
	}
	EPersonMeta result = {0};
	result.hasData = NotFound;
	return result;
}

EPersonMeta
FindPersonMeta(const Generation* aData, int aPerson) {
	for (size_t i = 0; i < aData->length; i++) {
		if (aData->data[i]->person->person == aPerson) {
			EPersonMeta result = {0};
			result.data = aData->data[i];
			result.hasData = True;
			return result;
		}
	}
	EPersonMeta result = {0};
	result.hasData = NotFound;
	return result;
}

#include <stdio.h>

void
FreePersonMeta(PersonMeta aPersonMeta) {
	FreePersonMetas(aPersonMeta.parents);
	FreePersonMetas(aPersonMeta.childs);
}

void
FreeFamily(Family aFamily) {
	FreePersonMetas(aFamily.parents);
	FreePersonMetas(aFamily.childs);
}
