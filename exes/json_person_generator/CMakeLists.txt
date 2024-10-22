cmake_minimum_required (VERSION 3.20)

add_executable(JsonPersonGenerator WIN32 "main.cpp")

target_include_directories(JsonPersonGenerator PRIVATE "src")

target_compile_features(JsonPersonGenerator PRIVATE cxx_std_17)

target_link_libraries(JsonPersonGenerator
	PRIVATE
		ImGuiAdapter
)
