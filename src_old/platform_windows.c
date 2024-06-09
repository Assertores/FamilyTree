#include "platform.h"

#include <windows.h>

Path
GetCurrentWorkingDirectory() {
	TCHAR buffer[MAX_PATH];
	DWORD result = GetCurrentDirectory(MAX_PATH, buffer); // TODO: W version verwenden
	if (result == 0) {
		return CreatePath(FromCString(""));
	}
	return CreatePath(FromCString(buffer));
}

void
PlayFile(Path aFile) {
	for (size_t i = 0; i < aFile.string.length; i++) {
		if (aFile.string.c_str[i] == '/') {
			aFile.string.c_str[i] = '\\';
		}
	}
	system(aFile.string.c_str); // NOLINT
	FreePath(aFile);
}

Bool
DetectAbsolutPath(const String* aPath) {
	for (size_t i = 0; i < aPath->length; i++) {
		if (aPath->c_str[i] == ':') {
			return True;
		}
		if (aPath->c_str[i] == '\\' || aPath->c_str[i] == '/') {
			return False;
		}
	}
	return False;
}