cmake_minimum_required (VERSION 3.20)

add_executable(FamilyTreeGui EXCLUDE_FROM_ALL "main.cpp")

target_sources(FamilyTreeGui
	PRIVATE
	"src/imgui_adapter.hpp"
	"src/view.cpp"
	"src/view.hpp"
	"src/windows_adapter.cpp"
	"src/windows_adapter.hpp"
)

target_include_directories(FamilyTreeGui PRIVATE "src")

target_compile_features(FamilyTreeGui PRIVATE cxx_std_17)

target_link_libraries(FamilyTreeGui
	PRIVATE
		FT::LibFamilyTreeStatic
		ImGui
)
