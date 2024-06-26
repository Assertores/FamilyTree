#include "trace.hpp"

ITrace*
AbstractTrace_CreateSubTrace(ITrace* aThis, const char* aSubtraceName) {
	return *((C_Trace*)aThis)->myThis->CreateSubTrace(aSubtraceName);
}

void
AbstractTrace_AddEvent(ITrace* aThis, const char* aEvent) {
	((C_Trace*)aThis)->myThis->AddEvent(aEvent);
}

void
AbstractTrace_Succeed(ITrace* aThis) {
	((C_Trace*)aThis)->myThis->Succeed();
}

void
AbstractTrace_Fail(ITrace* aThis, const char* aReason) {
	((C_Trace*)aThis)->myThis->Fail(aReason);
}

void
AbstractTrace_Free(ITrace* aThis) {}

AbstractTrace::AbstractTrace() {
	myInterface.myInterface.CreateSubTrace = AbstractTrace_CreateSubTrace;
	myInterface.myInterface.AddEvent = AbstractTrace_AddEvent;
	myInterface.myInterface.Succeed = AbstractTrace_Succeed;
	myInterface.myInterface.Fail = AbstractTrace_Fail;
	myInterface.myInterface.Free = AbstractTrace_Free;
	myInterface.myThis = this;
}

AbstractTrace::operator ITrace*() { return &myInterface.myInterface; }
