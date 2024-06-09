#ifndef GRAPH_H
#define GRAPH_H
#include "graph_types.h"

ArrPersonMetas CreateMetaData(Graph aGraph);
void ResolveGraph(ArrPersonMetas* aGraph);
Generations CreateGenerations(ArrPersonMetas* aMeta);
void AddProxys(const Generation* aTopGeneration, Generation* aBottomGeneration);
Familys CreateFamilies(Generation aTopGeneration, Generation aBottomGeneration);

void CalculateBriefWidth(PersonMeta* aPerson);

#endif // GRAPH_H