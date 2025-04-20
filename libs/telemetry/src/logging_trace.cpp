#include "logging_trace.hpp"

#include <fstream>

namespace tel {
std::string
Now() {
	auto now = std::chrono::system_clock::now().time_since_epoch();
	return std::to_string(now.count());
}

LoggingTrace::LoggingTrace(
	std::string aName,
	uint32_t* aIndexHandler,
	std::filesystem::path aFolder)
	: LoggingTrace(
		  std::move(aName),
		  aIndexHandler,
		  aFolder / std::to_string(*aIndexHandler),
		  locUseInternalConstructor) {
	myFile << std::flush;
}

LoggingTrace::LoggingTrace(
	std::string aName,
	uint32_t aOrigin,
	uint32_t* aIndexHandler,
	std::filesystem::path aFolder)
	: LoggingTrace(std::move(aName), aIndexHandler, std::move(aFolder), locUseInternalConstructor) {
	myFile << Now() << "|o|" << std::to_string(aOrigin) << '\n' << std::flush;
}

std::shared_ptr<Trace>
LoggingTrace::CreateSubTrace(std::string aSubtraceName) {
	myFile << Now() << "|t|" << aSubtraceName << '\n' << std::flush;
	return std::make_shared<LoggingTrace>(aSubtraceName, myIndex, myIndexHandler, myFolder);
}

void
LoggingTrace::AddEvent(std::string aEvent) {
	myFile << Now() << "|e|" << aEvent << '\n' << std::flush;
}

void
LoggingTrace::Succeed() {
	myFile << Now() << "|S|\n" << std::flush;
}

void
LoggingTrace::Fail(std::string aReason) {
	myFile << Now() << "|F|\n" << aReason << std::flush;
}

LoggingTrace::LoggingTrace(
	std::string aName,
	uint32_t* aIndexHandler,
	std::filesystem::path aFolder,
	Internal /*aIgnored*/)
	: myIndexHandler(aIndexHandler)
	, myIndex(*myIndexHandler)
	, myFolder(aFolder) {
	std::filesystem::create_directories(aFolder);
	myFile = std::ofstream(aFolder / (std::to_string(*aIndexHandler) + ".trace"));
	myFile << Now() << "| |" << aName << '\n';
	++*myIndexHandler;
}
} // namespace tel
