cmake_minimum_required (VERSION 3.20)

add_executable(FamilyTreeGui WIN32 "main.cpp")

target_sources(FamilyTreeGui
	PRIVATE
		"src/detailed_view.cpp"
		"src/detailed_view.hpp"
		"src/global_trace.hpp"
		"src/list_view.cpp"
		"src/list_view.hpp"
		"src/on_frame_trace_decorator.cpp"
		"src/on_frame_trace_decorator.hpp"
		"src/search_view.cpp"
		"src/search_view.hpp"
		"src/tree_view.cpp"
		"src/tree_view.hpp"
		"src/view.hpp"
)

if(WIN32)
	target_sources(FamilyTreeGui
		PRIVATE
			"src/setup_view.cpp"
			"src/setup_view.hpp"
	)
endif(WIN32)

target_include_directories(FamilyTreeGui PRIVATE "src")

target_compile_features(FamilyTreeGui PRIVATE cxx_std_17)

target_link_libraries(FamilyTreeGui
	PRIVATE
		FamilyTreeAdapter
		ImGuiAdapter
		Telemetry
)
