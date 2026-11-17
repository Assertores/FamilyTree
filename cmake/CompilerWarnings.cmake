include_guard(GLOBAL)

# Turns on all warnings for a target and treats them as errors (docs/decisions/adr_0011.md, Rule 10).
# The Library additionally gets -pedantic-errors, see library/CMakeLists.txt.
# The warning for a value of an enum that a switch does not handle is named explicitly, as the
# conventions rely on it (docs/conventions.md#error-handling): -Wswitch is part of -Wall, while
# MSVC's C4062 is off by default and is turned on at level 4 with /w44062.
function(ft_enable_warnings aTarget)
	if(MSVC)
		target_compile_options(${aTarget} PRIVATE /W4 /w44062 /WX)
	else()
		target_compile_options(${aTarget} PRIVATE -Wall -Wextra -Wswitch -Werror)
	endif()
endfunction()
