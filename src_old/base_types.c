#include "base_types.h"

#include "platform.h"

ArrayImpl(Path, FreePath);
Bool
EqualInt(const int* aLhs, const int* aRhs) {
	return *aLhs == *aRhs ? True : Different;
}
void FreeInt(int aInt) {}
SetImpl(int, EqualInt, FreeInt);

String
FromCString(const char* aString) {
	String result;
	result.length = strlen(aString);
	result.c_str = malloc((result.length + 1) * sizeof(char));
	memcpy(result.c_str, aString, result.length);
	result.c_str[result.length] = '\0';
	return result;
}

String
Concatinate(String aLhs, String aRhs) {
	String result = ConcatConst(aLhs, aRhs);

	FreeString(aRhs);
	return result;
}

String
ConcatConst(String aLhs, const String aRhs) {
	if(aRhs.length == 0){
		return aLhs;
	}
	String result;
	result.length = aLhs.length + aRhs.length;
	result.c_str = realloc(aLhs.c_str, (result.length + 1) * sizeof(char));
	memcpy(result.c_str + aLhs.length, aRhs.c_str, aRhs.length * sizeof(char));
	result.c_str[result.length] = '\0';
	return result;
}

String
ConcatLiteral(String aLhs, const char* aRhs) {
	String result;
	size_t rhsLength = strlen(aRhs);
	result.length = aLhs.length + rhsLength;
	result.c_str = realloc(aLhs.c_str, (result.length + 1) * sizeof(char));
	memcpy(result.c_str + aLhs.length, aRhs, rhsLength * sizeof(char));
	result.c_str[result.length] = '\0';
	return result;
}

void
FreeString(String aString) {
	if (aString.length > 0) {
		free(aString.c_str);
	}
}

String
ErrorToString(Bool aError) {
	switch (aError) {
	case True:
		return FromCString("No error");
	case False:
		return FromCString("False");
	case InternalError:
		return FromCString("A internal Error has occurred");
	case Duplicate:
		return FromCString("The provided item already exists");
	case Different:
		return FromCString("The provided item differes");
	case NotImplimented:
		return FromCString("Feature not yet implimented");
	case CantOpenFile:
		return FromCString("Requested file can not be opened");
	case MalformedInputFile:
		return FromCString("Provided file has unexpected syntax");
	case NotFound:
		return FromCString("Element not found");
	case OutOfRange:
		return FromCString("Request was out of acceptable Range");
	case EndOfFile:
		return FromCString("The end of the file has been reached");
	}
	return FromCString("Unrecogniced error type");
}

Path
CreatePath(String aString) {
	Path result = {0};
	result.isAbsolut = False;
	result.isDirectory = True;
	result.string = aString;

	result.isAbsolut = DetectAbsolutPath(&aString);
	size_t lastDot = 0;
	for (size_t i = 0; i < aString.length; i++) {
		if (result.string.c_str[i] == '\\') {
			result.string.c_str[i] = '/';
		}
		if (result.string.c_str[i] == '/') {
			result.directoryPart = i;
		}
		if (result.string.c_str[i] == '.') {
			lastDot = i;
		}
	}
	result.isDirectory = result.directoryPart > lastDot ? True : False;
	if (result.isDirectory == True) {
		result.directoryPart = result.string.length;
	}

	return result;
}

Path
AppendToPath(Path aPath, String aString, Bool aAddSeperator) {
	if (aAddSeperator == True) {
		aPath.string = Concatinate(aPath.string, FromCString("/"));
	}
	aPath.string = Concatinate(aPath.string, aString);

	return aPath;
}

Path
GetAbsolut(Path aPath, Path aRoot) {
	if (aPath.isAbsolut == True) {
		return aPath;
	}
	return AppendToPath(GetDirectoryPart(aRoot), aPath.string, True);
}
void
Normalice(Path* aPath) {
	// TODO: find `XX/..` and remove it.
}

Path
GetDirectoryPart(Path aPath) {
	if (aPath.isDirectory == True) {
		return aPath;
	}
	String result;
	result.c_str = aPath.string.c_str;
	result.length = aPath.directoryPart;
	result.c_str[aPath.directoryPart] = '\0';

	return CreatePath(result);
}

String
GetFilePart(Path aPath) {
	if (aPath.isDirectory == True) {
		FreePath(aPath);
		return FromCString("");
	}
	String result = FromCString(aPath.string.c_str + aPath.directoryPart);
	FreePath(aPath);
	return result;
}

void
FreePath(Path aPath) {
	FreeString(aPath.string);
}

Path CopyPath(Path aOriginal) {
	Path result;
	result.directoryPart = aOriginal.directoryPart;
	result.isAbsolut = aOriginal.isAbsolut;
	result.isDirectory = aOriginal.isDirectory;
	result.string = FromCString(aOriginal.string.c_str);
	return result;
}
