cmake_minimum_required (VERSION 3.20)

add_library(ImGuiAdapter STATIC)

target_sources(ImGuiAdapter
	PUBLIC
		"src/imgui_adapter.hpp"
	PRIVATE
		"src/windows_adapter.cpp"
		"src/windows_adapter.hpp"
)

target_include_directories(ImGuiAdapter PUBLIC "src")

target_compile_features(ImGuiAdapter PRIVATE cxx_std_17)

target_link_libraries(ImGuiAdapter
	PUBLIC
		ImGui
)

add_executable(FamilyTreeGui WIN32 "main.cpp")

target_sources(FamilyTreeGui
	PRIVATE
		"src/view.cpp"
		"src/view.hpp"
)

target_include_directories(FamilyTreeGui PRIVATE "src")

target_compile_features(FamilyTreeGui PRIVATE cxx_std_17)

target_link_libraries(FamilyTreeGui
	PRIVATE
		FT::LibFamilyTreeStatic
		ImGuiAdapter
)
