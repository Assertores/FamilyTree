#include "algorythms_test.hpp"
#include "api_tests.hpp"
#include "csv_relation_tests.hpp"
#include "json_person_tests.hpp"
#include "provider_composit_tests.hpp"

#ifndef NDEBUG
#include <crtdbg.h>
#endif

#include <iostream>

static bool
Run(const char* aTestName, bool (*aTest)()) {
	std::cout << "[TEST   ] ===== " << aTestName << '\n';
	auto result = false;
	try {
		result = aTest();
	} catch (...) {
		std::cout << "catched exeption.\n";
	}
	std::cout << "[" << aTestName << "] ///// " << (result ? "SUCCESS" : " FAILED") << '\n';
	return result;
}
#define RUN(aTest) Run(#aTest, &(aTest))

int
main(int argc, char** argv) { // NOLINT(bugprone-exception-escape)
	bool result = true;
	std::cout << "[ SUITE ] JSONPerson\n";
	result &= RUN(JSONPersonIsZeroIfNoFoldersExist);
	result &= RUN(JSONPersonIsAmountOfFolders);
	result &= RUN(CanRetreavePersonIds);
	result &= RUN(PersonDataCanBeRetreaved);
	result &= RUN(PlayMusicTriggersCorrectPathOnPlatform);
	result &= RUN(OpenImageTriggersCorrectPathOnPlatform);
	std::cout << "\n[ SUITE ] API\n";
	result &= RUN(CanRetreavePersonThroughAPI);
	result &= RUN(CanPlayMusicThroughAPI);
	result &= RUN(CanShowImagesThroughAPI);
	result &= RUN(CanRetreaveRelationsOfPerson);
	result &= RUN(CanRetreavePartners);
	result &= RUN(CanRetreaveSiblings);
	result &= RUN(CanRetreaveCommonParents);
	result &= RUN(CanRetreaveCommonChildren);
	std::cout << "\n[ SUITE ] CSVRelations\n";
	result &= RUN(CSVRelationsCanHandleEmptyFile);
	result &= RUN(CSVRelationsCanComputeCountOfAllUniqueIds);
	result &= RUN(CSVRelationsCanComputeAllUniqueIds);
	result &= RUN(CSVRelationsCanComputeRelationsCount);
	result &= RUN(CSVRelationsCanComputeRelationsOfPerson);
	std::cout << "\n[ SUITE ] DataProviderComposit\n";
	result &= RUN(ProviderCompositGetPersonDoesNotLeakInternalId);
	result &= RUN(ProviderCompositGetRelationsDoesNotLeakInternalId);
	std::cout << "\n[ SUITE ] Algorythems\n";
	result &= RUN(ChildRelationIsCorrectlyUnderstood);
	result &= RUN(ParentRelationIsCorrectlyUnderstood);
	result &= RUN(GrandParentAndChildRelationIsCorrectlyUnderstood);

#ifndef NDEBUG
	if (_CrtDumpMemoryLeaks() != 0) {
		std::cout << "\n!!!!! >> A memory leak was detected << !!!!!\n";
	}
#endif

	std::cout << "===== DONE =====\n";
	return result ? 0 : 1;
}