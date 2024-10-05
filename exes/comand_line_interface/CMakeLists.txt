cmake_minimum_required (VERSION 3.20)

add_executable(FamilyTreeCli "main.cpp")

target_sources(FamilyTreeCli
	PRIVATE
		"src/commands.cpp"
		"src/commands.hpp"
		"src/graph.cpp"
		"src/graph.hpp"
		"src/trace.cpp"
		"src/trace.hpp"
)

target_include_directories(FamilyTreeCli PRIVATE "src")

target_compile_features(FamilyTreeCli PRIVATE cxx_std_17)

target_link_libraries(FamilyTreeCli
	PRIVATE
		FT::LibFamilyTreeStatic
)
