#ifndef APPLICATION_H
#define APPLICATION_H
#include "application_types.h"
#include "base_types.h"

ERelations ReadInRelationsData(Path aPath);
EPerson ReadInPersonalData(Path aPath);

Bool PrintGraph(Graph aGraph);
#endif // APPLICATION_H
