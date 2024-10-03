#include "internal_types.h"

#include <stdlib.h>

ITrace*
NoOpTrace_CreateSubTrace(ITrace* aThis, const char* aSubtraceName) {
	return CreateNoOpTrace();
}

void
NoOpTrace_AddEvent(ITrace* aThis, const char* aEvent) {}

void
NoOpTrace_Succeed(ITrace* aThis) {}

void
NoOpTrace_Fail(ITrace* aThis, const char* aReason) {}

void
NoOpTrace_Free(ITrace* aThis) {
	free(aThis);
}

ITrace*
CreateNoOpTrace() {
	ITrace* result = calloc(1, sizeof(ITrace));

	result->CreateSubTrace = NoOpTrace_CreateSubTrace;
	result->AddEvent = NoOpTrace_AddEvent;
	result->Succeed = NoOpTrace_Succeed;
	result->Fail = NoOpTrace_Fail;
	result->Free = NoOpTrace_Free;

	return result;
}
