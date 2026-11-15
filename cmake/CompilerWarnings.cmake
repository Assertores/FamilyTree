include_guard(GLOBAL)

# Turns on all warnings for a target and treats them as errors (docs/decisions/adr_0011.md, Rule 10).
# The Library additionally gets -pedantic-errors, see library/CMakeLists.txt.
function(ft_enable_warnings aTarget)
	if(MSVC)
		target_compile_options(${aTarget} PRIVATE /W4 /WX)
	else()
		target_compile_options(${aTarget} PRIVATE -Wall -Wextra -Werror)
	endif()
endfunction()
