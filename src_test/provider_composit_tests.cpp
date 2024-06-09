#include "provider_composit_tests.hpp"

#include "auto_free.hpp"
#include "check.hpp"
#include "mock_data_provider.hpp"

#include <internal_types.h>

bool
ProviderCompositGetPersonDoesNotLeakInternalId() {
	MockDataProvider dataProvider;
	auto composit = CreateProviderComposit();
	ProviderComposit_AddDataProvider(composit, dataProvider);
	AutoFree iface = ProviderComposit_Cast(composit);

	auto person = iface->GetPerson(iface, 0);

	CHECK(person.id, 0);
	return true;
}

bool
ProviderCompositGetRelationsDoesNotLeakInternalId() {
	MockDataProvider dataProvider;
	auto composit = CreateProviderComposit();
	ProviderComposit_AddDataProvider(composit, dataProvider);
	AutoFree iface = ProviderComposit_Cast(composit);

	Relation relations[2];
	iface->GetAllRelationsOfId(iface, 0, relations);

	CHECK_EXCLUDE(relations[0].id1, 234, 34, 9754, 26);
	CHECK_EXCLUDE(relations[0].id2, 234, 34, 9754, 26);
	CHECK_EXCLUDE(relations[1].id1, 234, 34, 9754, 26);
	CHECK_EXCLUDE(relations[1].id2, 234, 34, 9754, 26);
	return true;
}
