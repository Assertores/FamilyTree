#ifndef PLATFORM_H
#define PLATFORM_H
#include "base_types.h"

Path GetCurrentWorkingDirectory();
void PlayFile(Path aFile);

Bool DetectAbsolutPath(const String* aPath);

#endif // PLATFORM_H