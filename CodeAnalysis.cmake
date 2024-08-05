include_guard()

# See following document for more information about static checks in CMake:
# - Hoffman, Bill (7 February 2018). "Static checks with CMake/CDash (iwyu, clang-tidy, lwyu, cpplint and cppcheck)".
#   https://blog.kitware.com/static-checks-with-cmake-cdash-iwyu-clang-tidy-lwyu-cpplint-and-cppcheck

if(NOT DEFINED CMAKE_CXX_CLANG_TIDY)
	find_program(CLANG_TIDY_EXECUTABLE clang-tidy
		DOC "Path to clang-tidy tool.")

	if(CLANG_TIDY_EXECUTABLE)
		message(STATUS "Found clang-tidy executable: ${CLANG_TIDY_EXECUTABLE}")
		set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
		set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE}" -p "${CMAKE_BINARY_DIR}"
			CACHE INTERNAL "Command line for the clang-tidy tool.")
	else()
		message(WARNING "clang-tidy not found")
	endif()

	mark_as_advanced(CLANG_TIDY_EXECUTABLE)
endif()
