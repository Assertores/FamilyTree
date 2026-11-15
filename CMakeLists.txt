cmake_minimum_required(VERSION 3.20)

# Builds the Library, the Applications and all tests. The Library itself does not need CMake
# (docs/decisions/adr_0002.md); this is a convenience for development and continuous integration.
project(
	FamilyTree
	VERSION 0.1.0
	LANGUAGES C CXX)

option(FT_BUILD_APPLICATIONS "Build the Applications and their tests; needs Qt 6" ON)
if(WIN32)
	set(FT_SANITIZERS_DEFAULT OFF)
else()
	set(FT_SANITIZERS_DEFAULT ON)
endif()
option(FT_SANITIZERS "Build everything with AddressSanitizer and LeakSanitizer"
	${FT_SANITIZERS_DEFAULT})
option(BUILD_SHARED_LIBS "Build the Library as a shared library instead of a static one" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CTest)
include(CompilerWarnings)
include(ClangTidy)
include(ClangFormat)

# The tests fail on any leak (docs/conventions.md). The sanitizers have to be on for the Library as
# well, so they are set before any target is defined.
if(FT_SANITIZERS)
	add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
	add_link_options(-fsanitize=address)
endif()

add_subdirectory(tests)
add_subdirectory(library)
if(FT_BUILD_APPLICATIONS)
	add_subdirectory(applications)
endif()
