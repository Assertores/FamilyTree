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

#define RUN(aTest)                          \
	printf("[TEST   ] ===== %s\n", #aTest); \
	printf("[%s] ///// %s\n", aTest() ? "SUCCESS" : " FAILED", #aTest);

int
main(int argc, char** argv) {
	printf("[ SUITE ] JSONPerson\n");
	RUN(JSONPersonIsZeroIfNoFoldersExist);
	RUN(JSONPersonIsAmountOfFolders);
	RUN(CanRetreavePersonIds);
	RUN(PersonDataCanBeRetreaved);
	RUN(PlayMusicTriggersCorrectPathOnPlatform);
	RUN(OpenImageTriggersCorrectPathOnPlatform);
	printf("\n[ SUITE ] API\n");
	RUN(CanRetreavePersonThroughAPI);
	RUN(CanPlayMusicThroughAPI);
	RUN(CanShowImagesThroughAPI);
	RUN(CanRetreaveRelationsOfPerson);
	printf("\n[ SUITE ] CSVRelations\n");
	RUN(CSVRelationsCanHandleEmptyFile);
	RUN(CSVRelationsCanComputeCountOfAllUniqueIds);
	RUN(CSVRelationsCanComputeAllUniqueIds);
	RUN(CSVRelationsCanComputeRelationsCount);
	RUN(CSVRelationsCanComputeRelationsOfPerson);
	printf("\n[ SUITE ] DataProviderComposit\n");
	RUN(ProviderCompositGetPersonDoesNotLeakInternalId);
	RUN(ProviderCompositGetRelationsDoesNotLeakInternalId);
	printf("\n[ SUITE ] Algorythems\n");
	RUN(ChildRelationIsCorrectlyUnderstood);
	RUN(ParentRelationIsCorrectlyUnderstood);
	RUN(GrandParentAndChildRelationIsCorrectlyUnderstood);

#ifndef NDEBUG
	if (_CrtDumpMemoryLeaks()) {
		printf("\n!!!!! >> A memory leak was detected << !!!!!\n");
	}
#endif
	return 0;
}