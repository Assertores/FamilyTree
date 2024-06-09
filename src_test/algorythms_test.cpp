#include "algorythms_test.hpp"

#include "algorythms.h"
#include "auto_free.hpp"
#include "check.hpp"
#include "mock_data_provider.hpp"

bool
ChildRelationIsCorrectlyUnderstood() {
	MockDataProvider data{};
	AutoFree<MetaData> metadata = CreateMetaData(data);

	auto result = ComputeRelativeGeneration(metadata, 234, 34);
	CHECK(result, 1);
	result = ComputeRelativeGeneration(metadata, 234, 26);
	CHECK(result, 1);
	result = ComputeRelativeGeneration(metadata, 34, 9754);
	CHECK(result, 1);
	return true;
}

bool
ParentRelationIsCorrectlyUnderstood() {
	MockDataProvider data{};
	AutoFree<MetaData> metadata = CreateMetaData(data);

	auto result = ComputeRelativeGeneration(metadata, 34, 234);
	CHECK(result, -1);
	result = ComputeRelativeGeneration(metadata, 26, 234);
	CHECK(result, -1);
	result = ComputeRelativeGeneration(metadata, 9754, 34);
	CHECK(result, -1);
	return true;
}

bool
GrandParentAndChildRelationIsCorrectlyUnderstood() {
	MockDataProvider data{};
	AutoFree<MetaData> metadata = CreateMetaData(data);

	auto result = ComputeRelativeGeneration(metadata, 234, 9754);
	CHECK(result, 2);
	result = ComputeRelativeGeneration(metadata, 9754, 234);
	CHECK(result, -2);
	return true;
}
