#include "provider_composit_tests.hpp"

#include "auto_free.hpp"
#include "check.hpp"
#include "mock_data_provider.hpp"

#include <internal_types.h>

#include <array>

bool
ProviderCompositGetPersonDoesNotLeakInternalId() {
	MockDataProvider dataProvider;
	auto* trace = CreateNoOpTrace();
	auto* composit = CreateProviderComposit(trace);
	ProviderComposit_AddDataProvider(composit, dataProvider, trace);
	AutoFree iface = ProviderComposit_Cast(composit);

	auto person = iface->GetPerson(iface, 0, trace);

	CHECK(person.id, 0); // NOLINT(readability-simplify-boolean-expr)
	return true;
}

bool
ProviderCompositGetRelationsDoesNotLeakInternalId() {
	MockDataProvider dataProvider;
	auto* trace = CreateNoOpTrace();
	auto* composit = CreateProviderComposit(trace);
	ProviderComposit_AddDataProvider(composit, dataProvider, trace);
	AutoFree iface = ProviderComposit_Cast(composit);

	std::array<Relation, 2> relations{};
	iface->GetAllRelationsOfId(iface, 0, relations.data(), trace);

	CHECK_EXCLUDE(relations[0].id1, 234, 34, 9754, 26);
	CHECK_EXCLUDE(relations[0].id2, 234, 34, 9754, 26);
	CHECK_EXCLUDE(relations[1].id1, 234, 34, 9754, 26);
	CHECK_EXCLUDE(relations[1].id2, 234, 34, 9754, 26);
	return true;
}
