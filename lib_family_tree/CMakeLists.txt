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
		"includes/family_tree/types.h"
	PRIVATE
		"src/algorythms.c"
		"src/algorythms.h"
		"src/context.c"
		"src/csv_relation.c"
		"src/forwarding_provider.c"
		"src/internal_types.h"
		"src/json_parser.c"
		"src/json_parser.h"
		"src/json_person.c"
		"src/noop_trace.c"
		"src/provider_composit.c"
		"src/shared_type_decorator.c"
)

if(WIN32)
	target_sources(${libName}
		PUBLIC
			"src/windows_platform.c"
	)
else()
	target_sources(${libName}
		PUBLIC
			"src/null_platform.c"
	)
endif()
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
