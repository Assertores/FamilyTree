include_guard(GLOBAL)

# The target documentation: runs Doxygen over the public header of the Library and writes HTML to
# <build folder>/documentation/html. Every warning, e.g. an undocumented function or parameter, is
# an error, so that the header stays fully documented (docs/requirements/req_0043.md).

find_package(Doxygen)

if(NOT DOXYGEN_FOUND)
	message(WARNING "Doxygen not found; the target documentation is missing")
	return()
endif()
message(STATUS "Doxygen: ${DOXYGEN_EXECUTABLE}")

set(DOXYGEN_PROJECT_NAME "Family Tree Library")
set(DOXYGEN_PROJECT_NUMBER "${PROJECT_VERSION}")
set(DOXYGEN_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/documentation")
set(DOXYGEN_STRIP_FROM_PATH "${PROJECT_SOURCE_DIR}/library/include")
set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C YES)
set(DOXYGEN_TYPEDEF_HIDES_STRUCT YES)
set(DOXYGEN_EXTRACT_ALL NO)
set(DOXYGEN_WARN_IF_UNDOCUMENTED YES)
set(DOXYGEN_WARN_NO_PARAMDOC YES)
set(DOXYGEN_WARN_AS_ERROR FAIL_ON_WARNINGS)
set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_TREEVIEW YES)
set(DOXYGEN_GENERATE_LATEX NO)
set(DOXYGEN_SHOW_USED_GROUPS YES)
set(DOXYGEN_QUIET YES)

doxygen_add_docs(
	documentation "${PROJECT_SOURCE_DIR}/library/include/family_tree/api.h"
	WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
	COMMENT "Generating the documentation of the Library")
