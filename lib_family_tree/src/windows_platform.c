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
	char string[2048];
	sprintf(string, "Get Folders for path %s", aPath);
	ITrace* trace = aTrace->CreateSubTrace(aTrace, string);

	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	char* sPath = malloc(strlen(aPath) + 5);
	char* result = calloc(2, sizeof(char));

	trace->AddEvent(trace, "Build Matcher Pattern for all Files in folder");
	// Specify a file mask. *.* = We want everything!
	sprintf(sPath, "%s\\*.*", aPath);

	if ((hFind = FindFirstFileA(sPath, &fdFile)) == INVALID_HANDLE_VALUE) {
		trace->Fail(trace, "Folder is empty");
		trace->Free(trace);
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
		sprintf(string, "found folder %s", fdFile.cFileName);
		trace->AddEvent(trace, string);

		size_t length = strlen(fdFile.cFileName) + 1;
		result = realloc(result, lastPos + length + 1);
		strcpy_s(result + lastPos, length, fdFile.cFileName);
		lastPos += length;

		result[lastPos - 1] = '\0';
		result[lastPos] = '\0';
	} while (FindNextFileA(hFind, &fdFile));

	trace->AddEvent(trace, "Close and Free");
	trace->Free(trace);
	FindClose(hFind);
	free(sPath);

	return result;
}

char*
WindowsPlatform_ReadFile(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	char backingString[2048];
	sprintf(backingString, "Read in File %s", aPath);
	ITrace* trace = aTrace->CreateSubTrace(aTrace, backingString);

	trace->AddEvent(trace, "Open file");
	FILE* f = NULL;
	fopen_s(&f, aPath, "r");
	if (f == NULL) {
		trace->Fail(trace, "Unable to open file");
		trace->Free(trace);
		return calloc(1, sizeof(char));
	}

	trace->AddEvent(trace, "Find size");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	trace->AddEvent(trace, "Allocate String and read in data");
	char* string = calloc(fsize + 1, sizeof(char));
	fread(string, fsize, 1, f);

	trace->AddEvent(trace, "Close file");
	fclose(f);

	trace->Free(trace);
	return string;
}

void
WindowsPlatform_FreeString(IPlatform* aThis, char* aString, ITrace* aTrace) {
	free(aString);
}

void
WindowsPlatform_OpenAudio(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	char backingString[2048];
	sprintf(backingString, "Play Audiofile %s", aPath);
	ITrace* trace = aTrace->CreateSubTrace(aTrace, backingString);

	size_t pathLength = strlen(aPath);

	trace->AddEvent(trace, "Enshure Path is in Windows format");
	char* path = malloc(pathLength + 1);
	strcpy_s(path, pathLength + 1, aPath);
	path[pathLength] = '\0';
	for (size_t i = 0; i < pathLength; i++) {
		if (path[i] == '/') {
			path[i] = '\\';
		}
	}

	trace->AddEvent(trace, "Open file via system call");
	ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWNORMAL);

	trace->Free(trace);
	free(path);
}

void
WindowsPlatform_OpenImage(IPlatform* aThis, const char* aPath, ITrace* aTrace) {
	char string[2048];
	sprintf(string, "Open Image %s", aPath);
	ITrace* trace = aTrace->CreateSubTrace(aTrace, string);

	size_t pathLength = strlen(aPath);

	trace->AddEvent(trace, "Enshure Path is in Windows format");
	char* path = malloc(pathLength + 1);
	strcpy_s(path, pathLength + 1, aPath);
	path[pathLength] = '\0';
	for (size_t i = 0; i < pathLength; i++) {
		if (path[i] == '/') {
			path[i] = '\\';
		}
	}

	trace->AddEvent(trace, "Show file via system call");
	ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWNORMAL);

	trace->Free(trace);
	free(path);
}

void
WindowsPlatform_Free(IPlatform* aThis, ITrace* aTrace) {
	free(aThis);
}

IPlatform*
CreatePlatform(ITrace* aTrace) {
	ITrace* trace = aTrace->CreateSubTrace(aTrace, "Construct Windows Platform");
	trace->AddEvent(trace, "Allocate memory");
	WindowsPlatform* result = calloc(1, sizeof(WindowsPlatform));

	trace->AddEvent(trace, "Set Dispatch Table");
	result->i.Copy = WindowsPlatform_Copy;
	result->i.GetFolders = WindowsPlatform_GetFolders;
	result->i.ReadFile = WindowsPlatform_ReadFile;
	result->i.FreeString = WindowsPlatform_FreeString;
	result->i.OpenAudio = WindowsPlatform_OpenAudio;
	result->i.OpenImage = WindowsPlatform_OpenImage;
	result->i.Free = WindowsPlatform_Free;

	return &result->i;
}