#ifndef FT_INTERNAL_TYPES_H
#define FT_INTERNAL_TYPES_H
#ifdef __cplusplus
extern "C" {
#endif

#include "family_tree/types.h"

typedef struct ProviderComposit ProviderComposit;

ProviderComposit* CreateProviderComposit(ITrace* aTrace);
IDataProvider*
CreateForwardingProvider(IRelationals* aRelations, IPersonals* aPersonals, ITrace* aTrace);
IRelationals* CreateCSVRelation(const char* aPath, IPlatform* aPlatform, ITrace* aTrace);
IPersonals* CreateJSONPerson(const char* aPath, IPlatform* aPlatform, ITrace* aTrace);
IPlatform* CreatePlatform(ITrace* aTrace);
ITrace* CreateNoOpTrace();

void
ProviderComposit_AddDataProvider(ProviderComposit* aThis, IDataProvider* aProvider, ITrace* aTrace);
IDataProvider* ProviderComposit_Cast(ProviderComposit* aThis);

IDataProvider* CreateSharedDataProviderDecorator(IDataProvider* aElement, ITrace* aTrace);
IRelationals* CreateSharedRelationalsDecorator(IRelationals* aElement, ITrace* aTrace);
IPersonals* CreateSharedPersonalsDecorator(IPersonals* aElement, ITrace* aTrace);
IPlatform* CreateSharedPlatformDecorator(IPlatform* aElement, ITrace* aTrace);

#ifdef __cplusplus
}
#endif
#endif // FT_INTERNAL_TYPES_H