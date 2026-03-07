cmake_minimum_required (VERSION 3.20)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

add_library(ImGui)

target_sources(ImGui
	PUBLIC
		"imgui/imgui.h"
		"imgui/backends/imgui_impl_opengl3.h"
		"imgui/backends/imgui_impl_sdl2.h"
		"imgui/backends/imgui_impl_vulkan.h"
	PRIVATE
		"imgui/backends/imgui_impl_opengl3.cpp"
		"imgui/backends/imgui_impl_sdl2.cpp"
		"imgui/backends/imgui_impl_vulkan.cpp"
		"imgui/imconfig.h"
		"imgui/imgui_draw.cpp"
		"imgui/imgui_internal.h"
		"imgui/imgui_tables.cpp"
		"imgui/imgui_widgets.cpp"
		"imgui/imgui.cpp"
		"imgui/imstb_rectpack.h"
		"imgui/imstb_textedit.h"
		"imgui/imstb_truetype.h"
)

if(WIN32)
	target_sources(ImGui
		PUBLIC
			"imgui/backends/imgui_impl_win32.h"
		PRIVATE
			"imgui/backends/imgui_impl_win32.cpp"
	)
endif(WIN32)

target_include_directories(ImGui
	PUBLIC
		"imgui"
		"imgui/backends")


if(WIN32)
target_link_libraries(ImGui
	PRIVATE
		OpenGL::GL
)
else(WIN32)
	include_directories(ImGui ${SDL2_INCLUDE_DIRS})
	target_link_libraries(ImGui
		${SDL2_LIBRARIES}
		Vulkan::Vulkan
	)
endif(WIN32)

target_compile_features(ImGui PRIVATE cxx_std_17)
