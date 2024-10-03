cmake_minimum_required (VERSION 3.20)

project ("FamilyTree" LANGUAGES C CXX)

include(CTest)
include("CodeAnalysis.cmake")

add_subdirectory("lib_family_tree")
add_subdirectory("exes/comand_line_interface")
