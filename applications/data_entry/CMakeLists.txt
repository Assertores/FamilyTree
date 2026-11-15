# The Data Entry Tool. Only its Person File reader and writer exists so far; the form is added here
# once it is written.

# The Person File reader and writer, without the form, so that the tests can use it.
add_library(DataEntryDocument STATIC "src/person_file_document.cpp" "src/person_file_document.hpp")
target_include_directories(DataEntryDocument PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(DataEntryDocument PRIVATE Qt6::Core)
ft_enable_warnings(DataEntryDocument)

if(BUILD_TESTING)
	add_executable(
		DataEntryTests
		"tests/main.cpp"
		"tests/person_file_document_tests.cpp"
		"tests/person_file_document_tests.hpp")

	# The tests read the saved Person Files back with the Library.
	target_link_libraries(DataEntryTests PRIVATE DataEntryDocument FamilyTree::TestSupport)
	ft_enable_warnings(DataEntryTests)
	ft_clang_tidy_for_tests(DataEntryTests)

	add_test(NAME DataEntryTests COMMAND DataEntryTests)
endif()
