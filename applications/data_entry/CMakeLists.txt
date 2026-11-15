# The Data Entry Tool. Only the tests of its Person File reader and writer exist so far; the
# Application itself and the source files in src/ are added here as they are written.

if(BUILD_TESTING)
	add_executable(
		DataEntryTests
		"tests/main.cpp"
		"tests/person_file_document_tests.cpp"
		"tests/person_file_document_tests.hpp"
		"src/person_file_document.hpp")

	target_include_directories(DataEntryTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
	# The tests read the saved Person Files back with the Library.
	target_link_libraries(
		DataEntryTests
		PRIVATE FamilyTree::FamilyTree
				FamilyTree::TestFramework
				Qt6::Core)
	ft_enable_warnings(DataEntryTests)
	ft_clang_tidy_for_tests(DataEntryTests)

	add_test(NAME DataEntryTests COMMAND DataEntryTests)
endif()
