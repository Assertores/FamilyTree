// Default Platform: listing folders, checking files and reading files, and opening Media with the
// programs of the operating system (docs/glossary.md#platform, docs/decisions/adr_0006.md). It
// knows nothing about Questions; the Builder decides what to ask it. Stubs: every function returns
// FT_ERROR_NOT_SUPPORTED until its work package implements it.

#include "internal.h"

#include <family_tree/api.h>
#include <stdbool.h>
#include <stddef.h>

// The stubs write no output yet; remove this block with the last stub.
// NOLINTBEGIN(readability-non-const-parameter)

FT_Result
PlatformListFolders(
	const char* aPath,
	Arena* aArena,
	const char* const** aOutNames,
	size_t* aOutCount,
	FT_Trace* aTrace) {
	(void)aArena;
	(void)aOutNames;
	(void)aOutCount;
	TraceError(aTrace, "there is no Default Platform yet to list the folders of %s", aPath);
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
PlatformFileExists(const char* aPath, bool* aOutExists, FT_Trace* aTrace) {
	(void)aOutExists;
	TraceError(aTrace, "there is no Default Platform yet to check %s", aPath);
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
PlatformReadFile(
	const char* aPath,
	Arena* aArena,
	char** aOutContent,
	bool* aOutFound,
	FT_Trace* aTrace) {
	(void)aArena;
	(void)aOutContent;
	(void)aOutFound;
	TraceError(aTrace, "there is no Default Platform yet to read %s", aPath);
	return FT_ERROR_NOT_SUPPORTED;
}

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
