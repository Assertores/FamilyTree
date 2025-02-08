cmake_minimum_required (VERSION 3.20)

add_library(ImGuiAdapter STATIC)

target_sources(ImGuiAdapter
	PUBLIC
		"src/imgui_adapter.hpp"
)

if(WIN32)
	target_sources(ImGuiAdapter
		PRIVATE
			"src/windows_adapter.cpp"
			"src/windows_adapter.hpp"
	)
else(WIN32)
	target_sources(ImGuiAdapter
		PRIVATE
			"src/linux_adapter.cpp"
			"src/linux_adapter.hpp"
	)
endif(WIN32)

target_include_directories(ImGuiAdapter PUBLIC "src")

target_compile_features(ImGuiAdapter PRIVATE cxx_std_17)

target_link_libraries(ImGuiAdapter
	PUBLIC
		ImGui
)

add_library(FamilyTreeAdapter STATIC)

target_sources(FamilyTreeAdapter
	PUBLIC
		"src/context_adapter.hpp"
	PRIVATE
		"src/context_adapter.cpp"
)

target_include_directories(FamilyTreeAdapter PUBLIC "src")

target_compile_features(FamilyTreeAdapter PRIVATE cxx_std_17)

target_link_libraries(FamilyTreeAdapter
	PUBLIC
		FT::LibFamilyTreeStatic
)

add_executable(FamilyTreeGui WIN32 "main.cpp")

target_sources(FamilyTreeGui
	PRIVATE
		"src/detailed_view.cpp"
		"src/detailed_view.hpp"
		"src/list_view.cpp"
		"src/list_view.hpp"
		"src/search_view.cpp"
		"src/search_view.hpp"
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
)
