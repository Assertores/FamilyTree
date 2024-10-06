#include "algorythms_test.hpp"
#include "api_tests.hpp"
#include "csv_relation_tests.hpp"
#include "json_parser_tests.hpp"
#include "json_person_tests.hpp"
#include "provider_composit_tests.hpp"

#ifndef NDEBUG
#include <crtdbg.h>
#endif

#include <csignal>
#include <iostream>
#include <string>

class Exept : public std::exception {
public:
	explicit Exept(std::string aReason)
		: myReason(std::move(aReason)) {}

	[[nodiscard]] char const* what() const noexcept override { return myReason.c_str(); }

private:
	std::string myReason;
};

void
SignalHandler(int aSignal) {
	throw Exept("Signal: " + std::to_string(aSignal));
}

static bool
Run(const char* aTestName, bool (*aTest)()) {
	std::cout << "[TEST   ] ===== " << aTestName << '\n';
	auto result = false;
	try {
		result = aTest();
	} catch (const std::exception& e) {
		std::cout << "catched exeption: " << e.what() << '\n';
	} catch (...) {
		std::cout << "catched exeption.\n";
	}
	std::cout << "[" << (result ? "SUCCESS" : " FAILED") << "]\n";
	return result;
}
#define RUN(aTest) Run(#aTest, &(aTest))

int
main(int argc, char** argv) { // NOLINT(bugprone-exception-escape)
	(void)signal(SIGSEGV, SignalHandler);

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
	std::cout << "\n[ SUITE ] JsonParser\n";
	result &= RUN(NullptrWillNotDoAnything);
	result &= RUN(EmptyStringWillNotDoAnything);
	result &= RUN(EmptyJsonWillNotCallParsingOnTheHandler);
	result &= RUN(AKeyIsSuccessfulyParsed);
	result &= RUN(StringValueIsNotConsideredAKey);
	result &= RUN(MultipleKeysAreDetected);
	result &= RUN(StringValueParserIsCalled);
	result &= RUN(IntParserIsNotCalledForStringValue);
	result &= RUN(IntValueParserIsCalled);
	result &= RUN(IgnoresWhitespaceCharacters);
	result &= RUN(CanDealWithArrayOfInts);
	result &= RUN(CallsKeyHandlerOnceForEveryElementInArray);
	result &= RUN(ArrayCanCountainStrings);
	result &= RUN(ArrayCanCountainMultipleStrings);
	result &= RUN(CanDealWithObjectInObject);
	result &= RUN(InnerKeysAreCalledOnInnerDispatchTable);
	result &= RUN(CanHandlerArrayOfObjects);

#ifndef NDEBUG
	if (_CrtDumpMemoryLeaks() != 0) {
		std::cout << "\n!!!!! >> A memory leak was detected << !!!!!\n";
	}
#endif

	std::cout << (result ? "All Succeeded" : "Failure detected") << '\n';
	std::cout << "===== DONE =====\n";
	return result ? 0 : 1;
}