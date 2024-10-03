cmake_minimum_required (VERSION 3.20)

add_executable(FamilyTree EXCLUDE_FROM_ALL "main.cpp")

target_sources(FamilyTree
	PRIVATE
		"src/commands.cpp"
		"src/commands.hpp"
		"src/graph.cpp"
		"src/graph.hpp"
		"src/trace.cpp"
		"src/trace.hpp"
)

target_include_directories(FamilyTree PRIVATE "src")

target_compile_features(FamilyTree PRIVATE cxx_std_17)

target_link_libraries(FamilyTree
	PRIVATE
		FT::LibFamilyTreeStatic
)
