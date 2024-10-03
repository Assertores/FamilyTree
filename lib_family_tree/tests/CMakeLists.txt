cmake_minimum_required (VERSION 3.20)

add_executable(FT_Test "main.cpp")

target_sources(FT_Test
	PRIVATE
		"src/algorythms_test.cpp"
		"src/algorythms_test.hpp"
		"src/api_tests.cpp"
		"src/api_tests.hpp"
		"src/auto_free.hpp"
		"src/check.hpp"
		"src/csv_relation_tests.cpp"
		"src/csv_relation_tests.hpp"
		"src/json_parser_tests.cpp"
		"src/json_parser_tests.hpp"
		"src/json_person_tests.cpp"
		"src/json_person_tests.hpp"
		"src/mock_data_provider.cpp"
		"src/mock_data_provider.hpp"
		"src/mock_platform.cpp"
		"src/mock_platform.hpp"
		"src/provider_composit_tests.cpp"
		"src/provider_composit_tests.hpp"
)

target_include_directories(FT_Test
    PRIVATE
        "src"
        "../src"
)

target_compile_features(FT_Test PRIVATE cxx_std_17)

target_link_libraries(FT_Test
	PRIVATE
		FT::LibFamilyTreeStatic
)

add_test(NAME FT_Test COMMAND FT_Test)
