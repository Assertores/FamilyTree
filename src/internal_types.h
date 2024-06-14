#ifndef FT_INTERNAL_TYPES_H
#define FT_INTERNAL_TYPES_H
#ifdef __cplusplus
extern "C" {
#endif

#include "family_tree/types.h"

typedef struct ProviderComposit ProviderComposit;

ProviderComposit* CreateProviderComposit();
IDataProvider* CreateForwardingProvider(IRelationals* aRelations, IPersonals* aPersonals);
IRelationals* CreateCSVRelation(const char* aPath, IPlatform* aPlatform);
IPersonals* CreateJSONPerson(const char* aPath, IPlatform* aPlatform);
IPlatform* CreatePlatform();
ITrace* CreateNoOpTrace();

void ProviderComposit_AddDataProvider(ProviderComposit* aThis, IDataProvider* aProvider);
IDataProvider* ProviderComposit_Cast(ProviderComposit* aThis);

IDataProvider* CreateSharedDataProviderDecorator(IDataProvider* aElement);
IRelationals* CreateSharedRelationalsDecorator(IRelationals* aElement);
IPersonals* CreateSharedPersonalsDecorator(IPersonals* aElement);
IPlatform* CreateSharedPlatformDecorator(IPlatform* aElement);

#ifdef __cplusplus
}
#endif
#endif // FT_INTERNAL_TYPES_H