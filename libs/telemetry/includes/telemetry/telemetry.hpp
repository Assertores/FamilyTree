#pragma once

#include <filesystem>
#include <memory>
#include <string>

struct ITrace;

namespace tel {
class Trace : public std::enable_shared_from_this<Trace> {
public:
	virtual ~Trace() = default;
	operator ITrace*(); // NOLINT(hicpp-explicit-conversions) to seamlessly interact with c API

	virtual std::shared_ptr<Trace> CreateSubTrace(std::string aSubtraceName) = 0;
	virtual void AddEvent(std::string aEvent) = 0;
	virtual void Succeed() = 0;
	virtual void Fail(std::string aReason) = 0;
};

class TelemetryFactory {
public:
	static TelemetryFactory& GetInstance();
	std::shared_ptr<Trace> CreateLoggingTrace(std::string aName);

private:
	TelemetryFactory();

	std::filesystem::path myFolder;
	uint32_t myIndex = 0;
};
} // namespace tel
