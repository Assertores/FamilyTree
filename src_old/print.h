#ifndef PRINT_H
#define PRINT_H
#include "base_types.h"
#include "graph.h"

EString PrintLineOfPersonBrief(const PersonMeta* aPerson, unsigned int aLineNumber);
EString PrintLineOfPersonDetailed(const Person* aPerson, unsigned int aLineNumber);
EString PrintLineOfPlumbing(
	const Generation* aTopGeneration,
	const Generation* aBottomGeneration,
	const Familys* aFamilys,
	unsigned int aLineNumber);

#endif // PRINT_H