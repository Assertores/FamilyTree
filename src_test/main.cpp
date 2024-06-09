#include "algorythms_test.hpp"
#include "api_tests.hpp"
#include "csv_relation_tests.hpp"
#include "json_person_tests.hpp"
#include "provider_composit_tests.hpp"

#ifndef NDEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <string.h>

static bool
Run(const char* aTestName, bool (*aTest)()) {
	printf("[TEST   ] ===== %s\n", aTestName);
	auto result = aTest();
	printf("[%s] ///// %s\n", result ? "SUCCESS" : " FAILED", aTestName);
	return result;
}
#define RUN(aTest) Run(#aTest, &aTest)

int
main(int argc, char** argv) {
	bool result = true;
	printf("[ SUITE ] JSONPerson\n");
	result &= RUN(JSONPersonIsZeroIfNoFoldersExist);
	result &= RUN(JSONPersonIsAmountOfFolders);
	result &= RUN(CanRetreavePersonIds);
	result &= RUN(PersonDataCanBeRetreaved);
	result &= RUN(PlayMusicTriggersCorrectPathOnPlatform);
	result &= RUN(OpenImageTriggersCorrectPathOnPlatform);
	printf("\n[ SUITE ] API\n");
	result &= RUN(CanRetreavePersonThroughAPI);
	result &= RUN(CanPlayMusicThroughAPI);
	result &= RUN(CanShowImagesThroughAPI);
	result &= RUN(CanRetreaveRelationsOfPerson);
	printf("\n[ SUITE ] CSVRelations\n");
	result &= RUN(CSVRelationsCanHandleEmptyFile);
	result &= RUN(CSVRelationsCanComputeCountOfAllUniqueIds);
	result &= RUN(CSVRelationsCanComputeAllUniqueIds);
	result &= RUN(CSVRelationsCanComputeRelationsCount);
	result &= RUN(CSVRelationsCanComputeRelationsOfPerson);
	printf("\n[ SUITE ] DataProviderComposit\n");
	result &= RUN(ProviderCompositGetPersonDoesNotLeakInternalId);
	result &= RUN(ProviderCompositGetRelationsDoesNotLeakInternalId);
	printf("\n[ SUITE ] Algorythems\n");
	result &= RUN(ChildRelationIsCorrectlyUnderstood);
	result &= RUN(ParentRelationIsCorrectlyUnderstood);
	result &= RUN(GrandParentAndChildRelationIsCorrectlyUnderstood);

#ifndef NDEBUG
	if (_CrtDumpMemoryLeaks()) {
		printf("\n!!!!! >> A memory leak was detected << !!!!!\n");
	}
#endif
	return result ? 0 : 1;
}