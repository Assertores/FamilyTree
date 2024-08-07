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

MetaData CreateMetaData(IDataProvider* aDataProvider, ITrace* aTrace);
void FreeMetaData(MetaData* aMetaData);
int ComputeRelativeGeneration(MetaData aMetaData, PersonId aId1, PersonId aId2, ITrace* aTrace);

size_t ComputePartnersMinimalCount(MetaData aMetaData, PersonId aId, ITrace* aTrace);
size_t ComputePartners(MetaData aMetaData, PersonId aId, PersonId* aOutPartners, ITrace* aTrace);

size_t ComputeSiblingsMinimalCount(MetaData aMetaData, PersonId aId, ITrace* aTrace);
size_t ComputeSiblings(MetaData aMetaData, PersonId aId, PersonId* aOutSiblings, ITrace* aTrace);

size_t GetParentsCount(MetaData aMetaData, PersonId aId, ITrace* aTrace);
void GetParents(MetaData aMetaData, PersonId aId, PersonId* aOutParents, ITrace* aTrace);
size_t IntersectParents(
	MetaData aMetaData,
	PersonId aId,
	size_t aParentCount,
	PersonId* aInOutParents,
	ITrace* aTrace);

size_t GetChildrenCount(MetaData aMetaData, PersonId aId, ITrace* aTrace);
void GetChildrens(MetaData aMetaData, PersonId aId, PersonId* aOutChildrens, ITrace* aTrace);
size_t IntersectChildrens(
	MetaData aMetaData,
	PersonId aId,
	size_t aChildrenCount,
	PersonId* aInOutChildrens,
	ITrace* aTrace);

#ifdef __cplusplus
}
#endif
#endif // FT_ALGORYTHMS_H
