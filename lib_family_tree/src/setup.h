#ifndef FT_SETUP_H
#define FT_SETUP_H

#include "context.h"

#include <family_tree/api.h>
#include <stddef.h>

typedef struct {
	Utf8_String rootPath;
	size_t rootPathStrlen;
	size_t currentPos;
	size_t foldernameCount;
	Utf8_String* foldernames;
	size_t allocatedStringCount;
	Utf8_String* allocatedStrings;
	int isRelations;
	Context* context;
} Internals;

ContextBuilder DoStart(Utf8_String aPath, DebuggingData* aDebug);
ContextBuilder DoContinue(ContextBuilder aPreviousStep, DebuggingData* aDebug);

#endif // FT_SETUP_H