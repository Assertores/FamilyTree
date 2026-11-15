// Context: releasing it (docs/decisions/adr_0003.md).
// Stubs: every function returns FT_ERROR_NOT_SUPPORTED until its work package implements it.

#include <family_tree/api.h>
#include <stdbool.h>
#include <stddef.h>

// The stubs write no output yet; remove this block with the last stub.
// NOLINTBEGIN(readability-non-const-parameter)

FT_Result
FT_Free(FT_Context* aContext, FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

// NOLINTEND(readability-non-const-parameter)
