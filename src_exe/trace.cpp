#include "trace.hpp"

#include <iostream>

// NOLINTBEGIN
ITrace*
AbstractTrace_CreateSubTrace(ITrace* aThis, const char* aSubtraceName) {
	return *((CTrace*)aThis)->myThis->CreateSubTrace(aSubtraceName);
}

void
AbstractTrace_AddEvent(ITrace* aThis, const char* aEvent) {
	((CTrace*)aThis)->myThis->AddEvent(aEvent);
}

void
AbstractTrace_Succeed(ITrace* aThis) {
	((CTrace*)aThis)->myThis->Succeed();
}

void
AbstractTrace_Fail(ITrace* aThis, const char* aReason) {
	((CTrace*)aThis)->myThis->Fail(aReason);
}

void
AbstractTrace_Free(ITrace* aThis) {
	((CTrace*)aThis)->myThis->Free();
}

AbstractTrace::AbstractTrace() {
	myInterface.myInterface.CreateSubTrace = AbstractTrace_CreateSubTrace;
	myInterface.myInterface.AddEvent = AbstractTrace_AddEvent;
	myInterface.myInterface.Succeed = AbstractTrace_Succeed;
	myInterface.myInterface.Fail = AbstractTrace_Fail;
	myInterface.myInterface.Free = AbstractTrace_Free;
	myInterface.myThis = this;
}
// NOLINTEND

AbstractTrace::operator ITrace*() { return &myInterface.myInterface; }

class PrintingTrace : public AbstractTrace {
public:
	PrintingTrace() = default;
	explicit PrintingTrace(std::string aPreviousIndent)
		: myIndent((std::move(aPreviousIndent) + indent)) {}

	AbstractTrace* CreateSubTrace(std::string_view aSubtraceName) override {
		std::cout << myIndent << "Subtrace: " << aSubtraceName << '\n';
		return new PrintingTrace(myIndent);
	}

	void AddEvent(std::string_view aEvent) override {
		std::cout << myIndent << "Event: " << aEvent << '\n';
	}

	void Succeed() override { std::cout << myIndent << "SUCCESS" << '\n'; }

	void Fail(std::string_view aReason) override {
		std::cout << myIndent << "FAILED: " << aReason << '\n';
	}

	void Free() override { delete this; }

private:
	std::string myIndent;

	static constexpr auto indent = " |  ";
};

std::shared_ptr<AbstractTrace>
AbstractTrace::CreatePrintingTrace() {
	return std::make_shared<PrintingTrace>();
}
