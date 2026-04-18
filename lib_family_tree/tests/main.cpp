#include "algorythms_test.hpp"
#include "api_tests.hpp"
#include "csv_relation_tests.hpp"
#include "json_parser_tests.hpp"
#include "json_person_tests.hpp"
#include "provider_composit_tests.hpp"

#include <iostream>

#if _WIN32
#include <csignal>
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
#else
#endif

#ifndef NDEBUG
#if _WIN32
#include <crtdbg.h>
#define LEAK_CHECK _CrtDumpMemoryLeaks() != 0
#else
#include <sanitizer/lsan_interface.h>
#define LEAK_CHECK __lsan_do_recoverable_leak_check() != 0
#endif
#else
#define LEAK_CHECK false
#endif

int
main(int argc, char** argv) { // NOLINT(bugprone-exception-escape)
#if _WIN32
	(void)signal(SIGSEGV, SignalHandler);
#endif

	bool result = true;
	result &= JSONPersonSuit();
	result &= APISuit();
	result &= CSVRelationSuit();
	result &= DataProviderSuit();
	result &= AlgorythemsSuit();
	result &= JsonParserSuit();

	if (LEAK_CHECK) {
		std::cout << "\n!!!!! >> A memory leak was detected << !!!!!\n";
	}

	std::cout << '\n' << (result ? "All Succeeded" : "Failure detected") << '\n';
	std::cout << "===== DONE =====\n";
	return result ? 0 : 1;
}
