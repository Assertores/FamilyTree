// Runs the unit tests of the Data Entry Tool, and checks for memory leaks at the end.

#include "person_file_document_tests.hpp"

#include <check.hpp>

int
main() {
	auto result = SETUP();
	result &= PersonFileDocumentSuite();
	return result;
}
