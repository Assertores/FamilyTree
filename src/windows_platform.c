#include "internal_types.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

typedef struct {
	// NOTE: windows.h macros interface away
	IPlatform i;
} WindowsPlatform;

IPlatform*
WindowsPlatform_Copy(IPlatform* aThis, ITrace* aTrace) {
	return aThis;
}

char*
WindowsPlatform_GetFolders(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	char* sPath = malloc(strlen(aPath) + 5);
	char* result = calloc(2, sizeof(char));

	// Specify a file mask. *.* = We want everything!
	sprintf(sPath, "%s\\*.*", aPath);

	if ((hFind = FindFirstFileA(sPath, &fdFile)) == INVALID_HANDLE_VALUE) {
		free(sPath);
		return result;
	}

	size_t lastPos = 0;
	do {
		// Find first file will always return "." and ".." as the first two directories.
		if (strcmp(fdFile.cFileName, ".") == 0 || strcmp(fdFile.cFileName, "..") == 0) {
			continue;
		}
		if (!(fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			continue;
		}
		size_t length = strlen(fdFile.cFileName) + 1;
		result = realloc(result, lastPos + length + 1);
		strcpy_s(result + lastPos, length, fdFile.cFileName);
		lastPos += length;

		result[lastPos - 1] = '\0';
		result[lastPos] = '\0';
	} while (FindNextFileA(hFind, &fdFile));

	FindClose(hFind);
	free(sPath);

	return result;
}

char*
WindowsPlatform_ReadFile(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	FILE* f = NULL;
	fopen_s(&f, aPath, "r");
	if (f == NULL) {
		return calloc(1, sizeof(char));
	}
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* string = calloc(fsize + 1, sizeof(char));
	fread(string, fsize, 1, f);
	fclose(f);

	return string;
}

void
WindowsPlatform_FreeString(IPlatform* aThis, char* aString, ITrace* aTrace) {
	free(aString);
}

void
WindowsPlatform_OpenAudio(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	size_t pathLength = strlen(aPath);

	char* path = malloc(pathLength + 1);
	strcpy_s(path, pathLength + 1, aPath);
	path[pathLength] = '\0';
	for (size_t i = 0; i < pathLength; i++) {
		if (path[i] == '/') {
			path[i] = '\\';
		}
	}

	system(path);

	free(path);
}

void
WindowsPlatform_OpenImage(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	size_t pathLength = strlen(aPath);

	char* path = malloc(pathLength + 1);
	strcpy_s(path, pathLength + 1, aPath);
	path[pathLength] = '\0';
	for (size_t i = 0; i < pathLength; i++) {
		if (path[i] == '/') {
			path[i] = '\\';
		}
	}

	system(path);

	free(path);
}

void
WindowsPlatform_Free(IPlatform* aThis, ITrace* aTrace) {
	free(aThis);
}

IPlatform*
CreatePlatform(ITrace* aTrace) {
	WindowsPlatform* result = calloc(1, sizeof(WindowsPlatform));

	result->i.Copy = WindowsPlatform_Copy;
	result->i.GetFolders = WindowsPlatform_GetFolders;
	result->i.ReadFile = WindowsPlatform_ReadFile;
	result->i.FreeString = WindowsPlatform_FreeString;
	result->i.OpenAudio = WindowsPlatform_OpenAudio;
	result->i.OpenImage = WindowsPlatform_OpenImage;
	result->i.Free = WindowsPlatform_Free;

	return &result->i;
}