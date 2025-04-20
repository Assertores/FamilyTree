#include "telemetry/telemetry.hpp"

#include "logging_trace.hpp"

#include <family_tree/types.h>

#include <chrono>

namespace tel {
struct CTrace {
	ITrace myInterface{};
	std::shared_ptr<Trace> myThis;
};

ITrace*
CreateSubTrace(ITrace* aThis, const char* aSubtraceName) {
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
	auto trace = ((CTrace*)aThis)->myThis->CreateSubTrace(aSubtraceName);
	return *trace;
}

void
AddEvent(ITrace* aThis, const char* aEvent) {
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
	((CTrace*)aThis)->myThis->AddEvent(aEvent);
}

void
Succeed(ITrace* aThis) {
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
	((CTrace*)aThis)->myThis->Succeed();
}

void
Fail(ITrace* aThis, const char* aReason) {
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
	((CTrace*)aThis)->myThis->Fail(aReason);
}

void
Free(ITrace* aThis) {
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
	auto* self = ((CTrace*)aThis);
	self->myThis.reset();
	delete self;
}

Trace::operator ITrace*() {
	auto* result = new CTrace();
	result->myThis = shared_from_this();
	result->myInterface.CreateSubTrace = &tel::CreateSubTrace;
	result->myInterface.AddEvent = &tel::AddEvent;
	result->myInterface.Succeed = &tel::Succeed;
	result->myInterface.Fail = &tel::Fail;
	result->myInterface.Free = &tel::Free;
	return &result->myInterface;
}

TelemetryFactory&
TelemetryFactory::GetInstance() {
	static TelemetryFactory instance;
	return instance;
}

TelemetryFactory::TelemetryFactory() {
	auto now = std::chrono::system_clock::now().time_since_epoch();
	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now);
	myFolder = std::filesystem::current_path() / "Logs" / std::to_string(seconds.count());
	std::filesystem::create_directories(myFolder);
}

std::shared_ptr<Trace>
TelemetryFactory::CreateLoggingTrace(std::string aName) {
	return std::make_shared<LoggingTrace>(aName, &myIndex, myFolder);
}
} // namespace tel
