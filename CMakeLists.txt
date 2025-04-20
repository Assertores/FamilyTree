cmake_minimum_required (VERSION 3.20)

project ("FamilyTree"
		LANGUAGES C CXX
		VERSION 0.1.0)

include(CTest)
include("CodeAnalysis.cmake")

add_subdirectory("lib_family_tree")
if(NOT DEFINED FT_CICD)
	add_subdirectory("external" EXCLUDE_FROM_ALL)
	add_subdirectory("exes" EXCLUDE_FROM_ALL)
	add_subdirectory("libs" EXCLUDE_FROM_ALL)
endif()
