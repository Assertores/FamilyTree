#include "on_frame_trace_decorator.hpp"

namespace ui {
OnFrameTraceDecorator::OnFrameTraceDecorator(std::shared_ptr<tel::Trace> aOriginalTrace)
	: myOriginalTrace(std::move(aOriginalTrace)) {}

void
OnFrameTraceDecorator::NextFrame() {
	myFrameCount++;
	mySubTrace.reset();
}

std::shared_ptr<tel::Trace>
OnFrameTraceDecorator::CreateSubTrace(std::string aSubtraceName) {
	PrivEnshureSubTrace();
	return mySubTrace->CreateSubTrace(aSubtraceName);
}

void
OnFrameTraceDecorator::AddEvent(std::string aEvent) {
	PrivEnshureSubTrace();
	mySubTrace->AddEvent(aEvent);
}

void
OnFrameTraceDecorator::Succeed() {
	PrivEnshureSubTrace();
	mySubTrace->Succeed();
}

void
OnFrameTraceDecorator::Fail(std::string aReason) {
	PrivEnshureSubTrace();
	mySubTrace->Fail(aReason);
}

void
OnFrameTraceDecorator::PrivEnshureSubTrace() {
	if (!mySubTrace) {
		mySubTrace = myOriginalTrace->CreateSubTrace("Frame " + std::to_string(myFrameCount));
	}
}
} // namespace ui