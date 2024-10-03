#pragma once

#include "algorythms.h"
#include "internal_types.h"

#include <family_tree/api.h>

template <class T>
class AutoFree {
public:
	// NOLINTNEXTLINE(hicpp-explicit-conversions) to seamlessly interact with c API
	AutoFree(T* aBacking)
		: myBacking(aBacking) {}
	~AutoFree() { myBacking->Free(myBacking, CreateNoOpTrace()); }

	AutoFree(const AutoFree&) = delete;
	AutoFree(AutoFree&&) noexcept = default;
	AutoFree& operator=(const AutoFree&) = delete;
	AutoFree& operator=(AutoFree&&) noexcept = default;

	// NOLINTNEXTLINE(hicpp-explicit-conversions) to seamlessly interact with c API
	operator T*() { return myBacking; }
	T* operator->() { return myBacking; }

private:
	T* myBacking;
};

template <>
class AutoFree<Context> {
public:
	// NOLINTNEXTLINE(hicpp-explicit-conversions) to seamlessly interact with c API
	AutoFree(Context* aBacking)
		: myBacking(aBacking) {}
	~AutoFree() { Free(myBacking, nullptr); }

	AutoFree(const AutoFree&) = delete;
	AutoFree(AutoFree&&) noexcept = default;
	AutoFree& operator=(const AutoFree&) = delete;
	AutoFree& operator=(AutoFree&&) noexcept = default;

	// NOLINTNEXTLINE(hicpp-explicit-conversions) to seamlessly interact with c API
	operator Context*() { return myBacking; }
	Context* operator->() { return myBacking; }

private:
	Context* myBacking;
};

template <>
class AutoFree<MetaData> {
public:
	// NOLINTNEXTLINE(hicpp-explicit-conversions) to seamlessly interact with c API
	AutoFree(MetaData aBacking)
		: myBacking(aBacking) {}
	~AutoFree() { FreeMetaData(&myBacking); }

	AutoFree(const AutoFree&) = delete;
	AutoFree(AutoFree&&) noexcept = default;
	AutoFree& operator=(const AutoFree&) = delete;
	AutoFree& operator=(AutoFree&&) noexcept = default;

	// NOLINTNEXTLINE(hicpp-explicit-conversions) to seamlessly interact with c API
	operator MetaData() { return myBacking; }
	MetaData operator->() { return myBacking; }

private:
	MetaData myBacking;
};
