#ifndef FT_ALGORYTHMS_H
#define FT_ALGORYTHMS_H
#ifdef __cplusplus
extern "C" {
#endif

#include "family_tree/types.h"

typedef struct PersonMeta_t PersonMeta;

typedef struct {
	size_t myPersonCount;
	PersonMeta* myPersons;
} MetaData;

MetaData CreateMetaData(IDataProvider* aDataProvider);
void FreeMetaData(MetaData* aMetaData);
int ComputeRelativeGeneration(MetaData aMetaData, PersonId aId1, PersonId aId2);

#ifdef __cplusplus
}
#endif
#endif // FT_ALGORYTHMS_H
