#pragma once

#include "algorythms.h"
#include "family_tree/api.h"

#include <stdlib.h>

template <class T>
class AutoFree {
public:
	AutoFree(T* aBacking)
		: myBacking(aBacking) {}
	~AutoFree() { myBacking->Free(myBacking); }

	operator T*() { return myBacking; }
	T* operator->() { return myBacking; }

private:
	T* myBacking;
};

template <>
class AutoFree<Context> {
public:
	AutoFree(Context* aBacking)
		: myBacking(aBacking) {}
	~AutoFree() { Free(myBacking); }

	operator Context*() { return myBacking; }
	Context* operator->() { return myBacking; }

private:
	Context* myBacking;
};

template <>
class AutoFree<MetaData> {
public:
	AutoFree(MetaData aBacking)
		: myBacking(aBacking) {}
	~AutoFree() { FreeMetaData(&myBacking); }

	operator MetaData() { return myBacking; }
	MetaData operator->() { return myBacking; }

private:
	MetaData myBacking;
};
