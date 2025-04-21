cmake_minimum_required (VERSION 3.20)

add_library(FamilyTreeAdapter STATIC)

target_sources(FamilyTreeAdapter
	PUBLIC
		"includes/familytree_adapter/context_adapter.hpp"
	PRIVATE
		"src/context_adapter.cpp"
)

target_include_directories(FamilyTreeAdapter PUBLIC "includes")

target_compile_features(FamilyTreeAdapter PRIVATE cxx_std_17)

target_link_libraries(FamilyTreeAdapter
	PUBLIC
		FT::LibFamilyTreeStatic
	PRIVATE
		Telemetry
)
