cmake_minimum_required (VERSION 3.20)

add_executable(TelemetryViewer  "main.cpp")

target_sources(TelemetryViewer
	PRIVATE
		"src/entries.cpp"
		"src/entries.hpp"
		"src/utils.cpp"
		"src/utils.hpp"
)

target_include_directories(TelemetryViewer PRIVATE "src")

target_compile_features(TelemetryViewer PRIVATE cxx_std_17)

target_link_libraries(TelemetryViewer
	PRIVATE
		ImGuiAdapter
)
