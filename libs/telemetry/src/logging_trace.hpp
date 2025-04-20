#pragma once

#include "telemetry/telemetry.hpp"

#include <fstream>

namespace tel {
class LoggingTrace : public Trace {
public:
	LoggingTrace(std::string aName, uint32_t* aIndexHandler, std::filesystem::path aFolder);

	// DONT USE!! INTERNAL ONLY
	LoggingTrace(
		std::string aName,
		uint32_t aOrigin,
		uint32_t* aIndexHandler,
		std::filesystem::path aFolder);

	std::shared_ptr<Trace> CreateSubTrace(std::string aSubtraceName) override;
	void AddEvent(std::string aEvent) override;
	void Succeed() override;
	void Fail(std::string aReason) override;

private:
	uint32_t* myIndexHandler;
	uint32_t myIndex;
	std::filesystem::path myFolder;
	std::ofstream myFile;

	using Internal = bool;
	static constexpr Internal locUseInternalConstructor = true;

	LoggingTrace(
		std::string aName,
		uint32_t* aIndexHandler,
		std::filesystem::path aFolder,
		Internal aIgnored);
};
} // namespace tel
