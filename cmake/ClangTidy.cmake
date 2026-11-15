include_guard(GLOBAL)

# Runs clang-tidy on every file while it is compiled. C is only used by the Library, so the C files
# get the configuration of the Library, and all C++ files the one of the Applications and tests
# (docs/conventions.md). The configuration files are passed explicitly, because the tests of the
# Library are C++ but live below library/.clang-tidy.

option(FT_CLANG_TIDY "Run clang-tidy while building" ON)

if(FT_CLANG_TIDY)
	find_program(FT_CLANG_TIDY_EXECUTABLE clang-tidy DOC "Path to clang-tidy.")
	mark_as_advanced(FT_CLANG_TIDY_EXECUTABLE)
	if(FT_CLANG_TIDY_EXECUTABLE)
		message(STATUS "clang-tidy: ${FT_CLANG_TIDY_EXECUTABLE}")
		set(CMAKE_C_CLANG_TIDY
			"${FT_CLANG_TIDY_EXECUTABLE}"
			"--config-file=${PROJECT_SOURCE_DIR}/library/.clang-tidy")
		set(CMAKE_CXX_CLANG_TIDY
			"${FT_CLANG_TIDY_EXECUTABLE}"
			"--config-file=${PROJECT_SOURCE_DIR}/.clang-tidy")
	else()
		message(WARNING "clang-tidy not found; the code is built without static analysis")
	endif()
endif()

# Test code checks results with CHECK, which returns early, and calls the C API with caller buffers
# and Person Ids as test data. These checks would demand a NOLINT on almost every line of a test.
set(FT_CLANG_TIDY_TEST_EXCLUSIONS
	-cppcoreguidelines-avoid-magic-numbers
	-readability-magic-numbers
	-cppcoreguidelines-avoid-c-arrays
	-hicpp-avoid-c-arrays
	-modernize-avoid-c-arrays
	-cppcoreguidelines-pro-bounds-array-to-pointer-decay
	-hicpp-no-array-decay
	-cppcoreguidelines-pro-bounds-avoid-unchecked-container-access
	-bugprone-unchecked-optional-access)

# Analyzes a test target with the checks above turned off.
function(ft_clang_tidy_for_tests aTarget)
	if(CMAKE_CXX_CLANG_TIDY)
		list(JOIN FT_CLANG_TIDY_TEST_EXCLUSIONS "," exclusions)
		set_target_properties(${aTarget} PROPERTIES CXX_CLANG_TIDY
			"${CMAKE_CXX_CLANG_TIDY};--checks=${exclusions}")
	endif()
endfunction()
