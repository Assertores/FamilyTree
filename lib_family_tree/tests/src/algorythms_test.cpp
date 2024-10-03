#include "algorythms_test.hpp"

#include "auto_free.hpp"
#include "check.hpp"
#include "mock_data_provider.hpp"

#include <algorythms.h>
#include <internal_types.h>

bool
ChildRelationIsCorrectlyUnderstood() {
	MockDataProvider data{};
	auto* trace = CreateNoOpTrace();
	AutoFree<MetaData> metadata = CreateMetaData(data, trace);

	auto result = ComputeRelativeGeneration(metadata, 1, 3, trace); // NOLINT
	CHECK(result, 1);
	result = ComputeRelativeGeneration(metadata, 5, 9, trace); // NOLINT
	CHECK(result, 2);
	result = ComputeRelativeGeneration(metadata, 1, 18, trace); // NOLINT
	CHECK(result, 7);
	return true;
}

bool
ParentRelationIsCorrectlyUnderstood() {
	MockDataProvider data{};
	auto* trace = CreateNoOpTrace();
	AutoFree<MetaData> metadata = CreateMetaData(data, trace);

	auto result = ComputeRelativeGeneration(metadata, 9, 1, trace); // NOLINT
	CHECK(result, -3);
	result = ComputeRelativeGeneration(metadata, 7, 5, trace); // NOLINT
	CHECK(result, -1);
	result = ComputeRelativeGeneration(metadata, 14, 10, trace); // NOLINT
	CHECK(result, -2);
	return true;
}

bool
GrandParentAndChildRelationIsCorrectlyUnderstood() {
	MockDataProvider data{};
	auto* trace = CreateNoOpTrace();
	AutoFree<MetaData> metadata = CreateMetaData(data, trace);

	auto result = ComputeRelativeGeneration(metadata, 1, 7, trace); // NOLINT
	CHECK(result, 2);
	result = ComputeRelativeGeneration(metadata, 11, 8, trace); // NOLINT
	CHECK(result, -2);
	return true;
}
