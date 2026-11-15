# Unit tests of the Library (docs/conventions.md).
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

target_link_libraries(FamilyTreeTests PRIVATE FamilyTree::FamilyTree FamilyTree::TestFramework)
ft_enable_warnings(FamilyTreeTests)
ft_clang_tidy_for_tests(FamilyTreeTests)

add_test(NAME FamilyTreeTests COMMAND FamilyTreeTests)
