cmake_minimum_required (VERSION 3.20)

add_library(ImGuiAdapter STATIC)

target_sources(ImGuiAdapter
	PUBLIC
		"includes/imgui_adapter/imgui_adapter.hpp"
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

target_include_directories(ImGuiAdapter PUBLIC "includes")

target_compile_features(ImGuiAdapter PRIVATE cxx_std_17)

target_link_libraries(ImGuiAdapter
	PUBLIC
		ImGui
)
