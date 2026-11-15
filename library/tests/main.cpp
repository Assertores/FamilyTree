// Runs every unit test suite of the Library, and checks for memory leaks at the end.

#include "builder_tests.hpp"
#include "context_tests.hpp"
#include "default_platform_tests.hpp"
#include "media_tests.hpp"
#include "parsers_tests.hpp"
#include "queries_tests.hpp"
#include "resolver_tests.hpp"
#include "trace_tests.hpp"

#include <check.hpp>

int
main() {
	auto result = SETUP();
	result &= TraceSuite();
	result &= BuilderSuite();
	result &= ParsersSuite();
	result &= ResolverSuite();
	result &= QueriesSuite();
	result &= MediaSuite();
	result &= ContextSuite();
	result &= DefaultPlatformSuite();
	return result;
}
