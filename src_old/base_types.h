#ifndef BASE_TYPES_H
#define BASE_TYPES_H
#include "generic_types.h"

#include <stddef.h>

typedef struct {
	char* c_str;
	size_t length;
} String;
String FromCString(const char* aString);
String Concatinate(String aLhs, String aRhs);
String ConcatConst(String aLhs, const String aRhs);
String ConcatLiteral(String aLhs, const char* aRhs);
void FreeString(String aString);

typedef enum {
	True = 0,
	False,
	InternalError,
	Duplicate,
	Different,
	NotImplimented,
	CantOpenFile,
	MalformedInputFile,
	NotFound,
	OutOfRange,
	EndOfFile,
} Bool;
String ErrorToString(Bool aError);

typedef struct {
	String string;
	Bool isAbsolut;
	size_t directoryPart;
	Bool isDirectory;
} Path;

Path CreatePath(String aString);
Path AppendToPath(Path aPath, String aString, Bool aAddSeperator);
Path GetAbsolut(Path aPath, Path aRoot);
Path CopyPath(Path aOriginal);
void Normalice(Path* aPath);

Path GetDirectoryPart(Path aPath);
String GetFilePart(Path aPath);
void FreePath(Path aPath);

Expected(String);
Array(Path);

Set(int);
#endif // BASE_TYPES_H
