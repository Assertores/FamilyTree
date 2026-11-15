// Default Platform: opening Media with the programs of the operating system
// (docs/decisions/adr_0006.md). Stubs: every function returns FT_ERROR_NOT_SUPPORTED until its work
// package implements it.

#include <family_tree/api.h>
#include <stdbool.h>
#include <stddef.h>

// The stubs write no output yet; remove this block with the last stub.
// NOLINTBEGIN(readability-non-const-parameter)

FT_Result
FT_DefaultShowPicture(const char* aPath, FT_Trace* aOutTrace) {
	(void)aPath;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_DefaultPlayRecording(const char* aPath, FT_Trace* aOutTrace) {
	(void)aPath;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_ShowPicturesOfPerson(const FT_Context* aContext, FT_PersonId aPerson, FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aPerson;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_PlayRecordingOfPerson(const FT_Context* aContext, FT_PersonId aPerson, FT_Trace* aOutTrace) {
	(void)aContext;
	(void)aPerson;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

// NOLINTEND(readability-non-const-parameter)
