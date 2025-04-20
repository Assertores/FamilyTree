#include "internal_types.h"

IPlatform*
CreatePlatform(ITrace* aTrace) {
	aTrace->Fail(aTrace, "!!Reached NULL Platform!!");
	return NULL;
}
