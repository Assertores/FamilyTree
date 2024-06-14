cmake_minimum_required (VERSION 3.20)

project ("FamilyTree" LANGUAGES C CXX)

include(CTest)

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

add_executable(FamilyTree EXCLUDE_FROM_ALL "src_exe/main.cpp")

target_sources(FamilyTree
	PRIVATE
		"src_exe/commands.cpp"
		"src_exe/commands.hpp"
		"src_exe/trace.cpp"
		"src_exe/trace.hpp"
)

target_compile_features(FamilyTree PRIVATE cxx_std_17)

target_link_libraries(FamilyTree
	PRIVATE
		FT::LibFamilyTreeStatic
)

add_executable(FT_Test "src_test/main.cpp")

target_sources(FT_Test
	PRIVATE
		"src_test/algorythms_test.cpp"
		"src_test/algorythms_test.hpp"
		"src_test/api_tests.cpp"
		"src_test/api_tests.hpp"
		"src_test/auto_free.hpp"
		"src_test/check.hpp"
		"src_test/csv_relation_tests.cpp"
		"src_test/csv_relation_tests.hpp"
		"src_test/json_person_tests.cpp"
		"src_test/json_person_tests.hpp"
		"src_test/mock_data_provider.cpp"
		"src_test/mock_data_provider.hpp"
		"src_test/mock_platform.cpp"
		"src_test/mock_platform.hpp"
		"src_test/provider_composit_tests.cpp"
		"src_test/provider_composit_tests.hpp"
)

target_include_directories(FT_Test PRIVATE "src")

target_compile_features(FT_Test PRIVATE cxx_std_17)

target_link_libraries(FT_Test
	PRIVATE
		FT::LibFamilyTreeStatic
)

add_test(NAME FT_Test COMMAND FT_Test)
