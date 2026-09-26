cmake_minimum_required (VERSION 3.20)

function(FT_declare libName)
add_library(FT::${libName} ALIAS ${libName})
target_include_directories(${libName}
	PUBLIC
		"includes"
)

target_sources(${libName}
	PUBLIC
		"includes/family_tree/api.h"
	PRIVATE
		"src/api.c"
		"src/context.h"
		"src/platform.h"
		"src/setup.h"
		"src/setup.c"
)
endfunction(FT_declare libName)

add_library(LibFamilyTree SHARED)
FT_declare(LibFamilyTree)
target_compile_definitions(LibFamilyTree
	PRIVATE
		FT_BUILD_LIBRARY
)
add_library(LibFamilyTreeStatic STATIC)
FT_declare(LibFamilyTreeStatic)
target_compile_definitions(LibFamilyTreeStatic
	PUBLIC
		FT_BUILD_STATIC
)

add_subdirectory("tests")
