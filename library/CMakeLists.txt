# The Family Tree Library, pure C99 (docs/requirements/req_0035.md). It also builds without CMake,
# with a single call of any C99 compiler (docs/decisions/adr_0002.md), so this file must not
# add anything the code depends on.

add_library(FamilyTree)
add_library(FamilyTree::FamilyTree ALIAS FamilyTree)

target_sources(
	FamilyTree
	PUBLIC "include/family_tree/api.h"
	PRIVATE "src/builder.c" "src/context.c" "src/default_platform.c" "src/queries.c")

target_include_directories(FamilyTree PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

set_target_properties(
	FamilyTree
	PROPERTIES C_STANDARD 99
			   C_STANDARD_REQUIRED ON
			   C_EXTENSIONS OFF
			   C_VISIBILITY_PRESET hidden
			   LINKER_LANGUAGE C)

ft_enable_warnings(FamilyTree)
if(NOT MSVC)
	target_compile_options(FamilyTree PRIVATE -pedantic-errors)
endif()

# The export macro FT_API in api.h.
if(BUILD_SHARED_LIBS)
	target_compile_definitions(
		FamilyTree
		PRIVATE FT_BUILD_SHARED
		INTERFACE FT_USE_SHARED)
endif()

if(BUILD_TESTING)
	add_subdirectory(tests)
endif()
