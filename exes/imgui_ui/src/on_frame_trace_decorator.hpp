#pragma once

#include <telemetry/telemetry.hpp>

namespace ui {
class OnFrameTraceDecorator : public tel::Trace {
public:
	explicit OnFrameTraceDecorator(std::shared_ptr<tel::Trace> aOriginalTrace);

	void NextFrame();

	std::shared_ptr<Trace> CreateSubTrace(std::string aSubtraceName) override;
	void AddEvent(std::string aEvent) override;
	void Succeed() override;
	void Fail(std::string aReason) override;

private:
	std::shared_ptr<tel::Trace> myOriginalTrace;
	uint64_t myFrameCount = 0;
	std::shared_ptr<tel::Trace> mySubTrace = nullptr;

	void PrivEnshureSubTrace();
};
} // namespace ui
