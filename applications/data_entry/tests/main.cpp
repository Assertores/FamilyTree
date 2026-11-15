// Runs the unit tests of the Data Entry Tool, and checks for memory leaks at the end.

#include "person_file_document_tests.hpp"

#include <check.hpp>

int
main(int argc, char** argv) { // NOLINT(misc-const-correctness): main must take char**
	auto result = SETUP(argc, argv);
	result &= PersonFileDocumentSuite();
	return result;
}
