include_guard(GLOBAL)

# Targets for clang-format, with the configuration .clang-format:
#   format         formats every C and C++ file of the project in place
#   check-format   fails if a file is not formatted; also runs as the test "Formatting"
# Spikes are not formatted.

find_program(FT_CLANG_FORMAT_EXECUTABLE clang-format DOC "Path to clang-format.")
mark_as_advanced(FT_CLANG_FORMAT_EXECUTABLE)

if(NOT FT_CLANG_FORMAT_EXECUTABLE)
	message(WARNING "clang-format not found; the targets format and check-format are missing")
	return()
endif()
message(STATUS "clang-format: ${FT_CLANG_FORMAT_EXECUTABLE}")

set(FT_FORMATTED_FOLDERS library applications tests)
set(FT_FORMATTED_PATTERNS)
foreach(folder IN LISTS FT_FORMATTED_FOLDERS)
	foreach(extension IN ITEMS c h cpp hpp)
		list(APPEND FT_FORMATTED_PATTERNS "${PROJECT_SOURCE_DIR}/${folder}/*.${extension}")
	endforeach()
endforeach()
file(GLOB_RECURSE FT_FORMATTED_FILES CONFIGURE_DEPENDS ${FT_FORMATTED_PATTERNS})

add_custom_target(
	format
	COMMAND "${FT_CLANG_FORMAT_EXECUTABLE}" -i ${FT_FORMATTED_FILES}
	WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
	COMMENT "Formatting the source files"
	VERBATIM)

add_custom_target(
	check-format
	COMMAND "${FT_CLANG_FORMAT_EXECUTABLE}" --dry-run --Werror ${FT_FORMATTED_FILES}
	WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
	COMMENT "Checking the formatting of the source files"
	VERBATIM)

if(BUILD_TESTING)
	add_test(
		NAME Formatting
		COMMAND "${FT_CLANG_FORMAT_EXECUTABLE}" --dry-run --Werror ${FT_FORMATTED_FILES}
		WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endif()
