# The own test framework (docs/decisions/adr_0014.md), a single header.
add_library(TestFramework INTERFACE)
add_library(FamilyTree::TestFramework ALIAS TestFramework)
target_include_directories(TestFramework INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/framework")
target_compile_features(TestFramework INTERFACE cxx_std_17)
