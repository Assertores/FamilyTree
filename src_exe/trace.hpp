#pragma once

#include <family_tree/types.h>

#include <string_view>

class AbstractTrace;
struct C_Trace {
	ITrace myInterface;
	AbstractTrace* myThis;
};

class AbstractTrace {
public:
	AbstractTrace();
	operator ITrace*();
	virtual ~AbstractTrace() = default;

	virtual AbstractTrace* CreateSubTrace(std::string_view aSubtraceName) = 0;
	virtual void AddEvent(std::string_view aEvent) = 0;
	virtual void Succeed() = 0;
	virtual void Fail(std::string_view aReason) = 0;

private:
	C_Trace myInterface;
};