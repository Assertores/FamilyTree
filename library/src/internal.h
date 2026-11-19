// Declarations shared by the source files of the Library; not part of the public API.
// See docs/decisions/adr_0018.md for the Trace writer and docs/decisions/adr_0019.md for the arena,
// the parsers and the index of Persons.
#ifndef FT_INTERNAL_H
#define FT_INTERNAL_H

#include <family_tree/api.h>
#include <stdbool.h>
#include <stddef.h>

// ----- Arena

typedef struct ArenaNode ArenaNode;

// Allocations that are freed together: as a whole, or back to a mark. Every allocation is one
// node of a list, newest first. Zero initialized, it is empty.
typedef struct {
	ArenaNode* last;
} Arena;

// A position in an Arena to reset it to: its newest allocation at the time.
typedef struct {
	ArenaNode* last;
} ArenaMark;

// Returns memory for any type, or NULL if memory ran out.
void* ArenaAllocate(Arena* aArena, size_t aSize);
// Returns a copy of a zero terminated text, or NULL if memory ran out.
char* ArenaCopyText(Arena* aArena, const char* aText);
ArenaMark ArenaGetMark(const Arena* aArena);
// Frees everything allocated after the mark.
void ArenaReset(Arena* aArena, ArenaMark aMark);
// Moves every allocation of aFrom into aTo, as if allocated there; aFrom is empty afterwards.
void ArenaMoveAll(Arena* aFrom, Arena* aTo);
// Frees every allocation; the Arena is empty afterwards.
void ArenaFree(Arena* aArena);

// ----- Internal result (docs/decisions/adr_0022.md)

// The result of an internal function that reports whether it did its task. It never leaves the
// Library: a failure without a public counterpart is handled by the caller. A value is added when a
// function needs it.
typedef enum {
	RESULT_OK,
	RESULT_OK_WITH_WARNINGS,
	RESULT_OUT_OF_MEMORY,
	RESULT_UNREADABLE, // a data file cannot be read into a record; the reason is in the Trace
} Result;

// The result of a public function for aResult, whose failures without a public counterpart were
// handled before.
FT_Result PublicResult(Result aResult);

// ----- Trace writer, the only code that touches an FT_Trace. Every function does nothing for a
// NULL Trace or a capacity of 0.

void TraceOpen(FT_Trace* aTrace, const char* aFunction, const char* aFormat, ...);
void TraceStep(FT_Trace* aTrace, const char* aFormat, ...);
void TraceWarning(FT_Trace* aTrace, const char* aFormat, ...);
void TraceError(FT_Trace* aTrace, const char* aFormat, ...);
// Closes the innermost Subtrace with the outcome of aResult, compacts it, and returns aResult.
FT_Result TraceClose(FT_Trace* aTrace, FT_Result aResult);
// Closes the innermost Subtrace like TraceClose, with the outcome of an internal result.
Result TraceCloseResult(FT_Trace* aTrace, Result aResult);
// Finishes the Trace before a public function returns, and returns aResult.
FT_Result TraceFinish(FT_Trace* aTrace, FT_Result aResult);
// Closes the Subtrace of a public function and finishes the Trace; returns aResult.
FT_Result TraceEndCall(FT_Trace* aTrace, FT_Result aResult);

// ----- Text

// The length of the valid UTF-8 sequence at aText, or 0 if it is not valid.
size_t Utf8SequenceLength(const char* aText);
bool IsValidUtf8(const char* aText);
// 3 if aText starts with a UTF-8 byte order mark, otherwise 0.
size_t ByteOrderMarkLength(const char* aText);
// Whether aText starts with the byte order mark of UTF-16 or UTF-32.
bool HasOtherByteOrderMark(const char* aText);
// Whether aText starts with aStart.
bool StartsWith(const char* aText, const char* aStart);
// Compares two texts, ignoring the case of the ASCII letters.
bool EqualsIgnoringCase(const char* aFirst, const char* aSecond);
// aFolder, aFirst and aSecond, if not NULL, joined by "/"; NULL if memory ran out.
char* JoinPath(Arena* aArena, const char* aFolder, const char* aFirst, const char* aSecond);
// Reads an optional "-" and at least one digit, and nothing else, as an integer.
bool ParseInteger(const char* aText, size_t aLength, FT_PersonId* aOut);

// ----- Parsers

// Parses a Person File in place, in a Subtrace of its own; aFile names it in warnings. The arrays
// of the record go into aArena, the values of an array go into aScratch while it is read. Returns
// RESULT_OK or RESULT_OK_WITH_WARNINGS with the record; RESULT_UNREADABLE if the file cannot be
// read into a record, e.g. as its Format Version is not supported, at which reading stops, with
// the reason in the Trace; or RESULT_OUT_OF_MEMORY.
Result ParsePersonFile(
	char* aText,
	const char* aFile,
	Arena* aScratch,
	Arena* aArena,
	FT_PersonRecord* aOutRecord,
	FT_Trace* aTrace);

// Parses a Relations File in place, like ParsePersonFile; the array of Relations goes into aArena,
// the array of its lines into aScratch. A row that cannot be read is skipped with a warning.
Result ParseRelationsFile(
	char* aText,
	const char* aFile,
	Arena* aScratch,
	Arena* aArena,
	FT_RelationList* aOutList,
	FT_Trace* aTrace);

// ----- Context

// A Person of the Context, with what belongs to it.
typedef struct {
	FT_Person person;
	const char* folder;
	const FT_Relation* relations;
	size_t relationCount;
} ContextPerson;

struct FT_Context {
	Arena arena;
	ContextPerson* persons; // sorted by Person Id
	size_t personCount;
};

// Returns a parsed Person with the Default String for every missing Detail. Its texts and arrays
// lie in the Arenas of the Builder (docs/decisions/adr_0020.md), so the missing texts within its
// arrays are completed in place.
FT_Person CompletePerson(const FT_Person* aPerson);

// Creates the Context from the Persons and Relations in aArena; the caller hands the Arena over to
// the Context. aScratch is working memory. Each Relation is copied to its Persons, with the
// Default String for every missing text; the texts it points to have to be kept by the caller.
FT_Result CreateContext(
	Arena* aArena,
	Arena* aScratch,
	const ContextPerson* aPersons,
	size_t aPersonCount,
	const FT_Relation* aRelations,
	size_t aRelationCount,
	FT_Context** aOutContext,
	FT_Trace* aTrace);

// The Person with the Person Id, or NULL.
const ContextPerson* FindPerson(const FT_Context* aContext, FT_PersonId aPerson);
// Whether aText is the Default String.
bool IsDefaultString(const char* aText);

// ----- Default Platform, the three operations of the Platform (docs/glossary.md#platform). Their
// results look like the answer of an Application after the Builder copied it.

// The names of the subfolders of aPath, allocated in aArena; NULL if there are none.
FT_Result PlatformListFolders(
	const char* aPath,
	Arena* aArena,
	const char* const** aOutNames,
	size_t* aOutCount,
	FT_Trace* aTrace);
// Whether the file aPath exists.
FT_Result PlatformFileExists(const char* aPath, bool* aOutExists, FT_Trace* aTrace);
// The content of the file aPath, zero terminated and allocated in aArena; aOutFound is false if
// the file cannot be read, and the content is NULL then.
FT_Result PlatformReadFile(
	const char* aPath,
	Arena* aArena,
	char** aOutContent,
	bool* aOutFound,
	FT_Trace* aTrace);

#endif // FT_INTERNAL_H
