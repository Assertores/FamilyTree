# Unit tests of the Library (docs/conventions.md).

# Helpers shared with the tests of the Data Entry Tool, included as <support/test_support.hpp>.
add_library(TestSupport INTERFACE)
add_library(FamilyTree::TestSupport ALIAS TestSupport)
target_include_directories(TestSupport INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(TestSupport INTERFACE FamilyTree::FamilyTree FamilyTree::TestFramework)

add_executable(
	FamilyTreeTests
	"main.cpp"
	"builder_tests.cpp"
	"builder_tests.hpp"
	"context_tests.cpp"
	"context_tests.hpp"
	"default_platform_tests.cpp"
	"default_platform_tests.hpp"
	"media_tests.cpp"
	"media_tests.hpp"
	"parsers_tests.cpp"
	"parsers_tests.hpp"
	"queries_tests.cpp"
	"queries_tests.hpp"
	"resolver_tests.cpp"
	"resolver_tests.hpp"
	"trace_tests.cpp"
	"trace_tests.hpp"
	"support/test_support.hpp")

target_link_libraries(FamilyTreeTests PRIVATE FamilyTree::TestSupport)
ft_enable_warnings(FamilyTreeTests)
ft_clang_tidy_for_tests(FamilyTreeTests)

add_test(NAME FamilyTreeTests COMMAND FamilyTreeTests)
