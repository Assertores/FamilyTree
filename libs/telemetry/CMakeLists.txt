cmake_minimum_required (VERSION 3.20)

add_library(Telemetry STATIC)

target_sources(Telemetry
	PUBLIC
		"includes/telemetry/telemetry.hpp"
	PRIVATE
		"src/logging_trace.cpp"
		"src/logging_trace.hpp"
		"src/telemetry.cpp"
)

target_include_directories(Telemetry PUBLIC "includes")

target_compile_features(Telemetry PRIVATE cxx_std_17)

target_link_libraries(Telemetry
	PRIVATE
		FT::LibFamilyTreeStatic
)
