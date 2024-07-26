#include "algorythms_test.hpp"

#include "auto_free.hpp"
#include "check.hpp"
#include "mock_data_provider.hpp"

#include <algorythms.h>
#include <internal_types.h>

bool
ChildRelationIsCorrectlyUnderstood() {
	MockDataProvider data{};
	auto trace = CreateNoOpTrace();
	AutoFree<MetaData> metadata = CreateMetaData(data, trace);

	auto result = ComputeRelativeGeneration(metadata, 234, 34, trace);
	CHECK(result, 1);
	result = ComputeRelativeGeneration(metadata, 234, 26, trace);
	CHECK(result, 1);
	result = ComputeRelativeGeneration(metadata, 34, 9754, trace);
	CHECK(result, 1);
	return true;
}

bool
ParentRelationIsCorrectlyUnderstood() {
	MockDataProvider data{};
	auto trace = CreateNoOpTrace();
	AutoFree<MetaData> metadata = CreateMetaData(data, trace);

	auto result = ComputeRelativeGeneration(metadata, 34, 234, trace);
	CHECK(result, -1);
	result = ComputeRelativeGeneration(metadata, 26, 234, trace);
	CHECK(result, -1);
	result = ComputeRelativeGeneration(metadata, 9754, 34, trace);
	CHECK(result, -1);
	return true;
}

bool
GrandParentAndChildRelationIsCorrectlyUnderstood() {
	MockDataProvider data{};
	auto trace = CreateNoOpTrace();
	AutoFree<MetaData> metadata = CreateMetaData(data, trace);

	auto result = ComputeRelativeGeneration(metadata, 234, 9754, trace);
	CHECK(result, 2);
	result = ComputeRelativeGeneration(metadata, 9754, 234, trace);
	CHECK(result, -2);
	return true;
}
