#include "platform.h"

#include <unistd.h>

Path
GetCurrentWorkingDirectory() {
	char buffer[_PC_PATH_MAX];
	if (getcwd(buffer, _PC_PATH_MAX)) {
		return CreatePath(FromCString(""));
	}
	return CreatePath(FromCString(buffer));
}

void
PlayFile(Path aFile) {
	system(aFile.string.c_str); // NOLINT
	FreePath(aFile);
}

Bool
DetectAbsolutPath(const String* aPath) {
	return aPath->c_str[0] == '/' ? True: False;
}