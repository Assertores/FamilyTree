// Builder: creating a Context through Questions (docs/decisions/adr_0005.md,
// docs/decisions/adr_0016.md). Stubs: every function returns FT_ERROR_NOT_SUPPORTED until its work
// package implements it.

#include <family_tree/api.h>
#include <stdbool.h>
#include <stddef.h>

// The stubs write no output yet; remove this block with the last stub.
// NOLINTBEGIN(readability-non-const-parameter)

FT_Result
FT_Create(const char* aDataFolder, FT_Context** aOutContext, FT_Trace* aOutTrace) {
	(void)aDataFolder;
	(void)aOutContext;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_StartCreate(
	const char* aDataFolder,
	FT_Builder** aOutBuilder,
	FT_Question* aOutQuestion,
	FT_Trace* aOutTrace) {
	(void)aDataFolder;
	(void)aOutBuilder;
	(void)aOutQuestion;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_Continue(
	FT_Builder* aBuilder,
	const FT_Answer* aAnswer,
	FT_Question* aOutQuestion,
	FT_Context** aOutContext,
	FT_Trace* aOutTrace) {
	(void)aBuilder;
	(void)aAnswer;
	(void)aOutQuestion;
	(void)aOutContext;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

FT_Result
FT_CancelCreate(FT_Builder* aBuilder, FT_Trace* aOutTrace) {
	(void)aBuilder;
	(void)aOutTrace;
	return FT_ERROR_NOT_SUPPORTED;
}

// NOLINTEND(readability-non-const-parameter)
