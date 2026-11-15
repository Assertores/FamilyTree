// Unit tests of the Trace writer: the format of docs/decisions/adr_0007.md, and working without a
// Trace.

#include "trace_tests.hpp"

#include "support/test_support.hpp"

#include <check.hpp>
#include <family_tree/api.h>

#include <cstddef>
#include <string>
#include <vector>

namespace {

using namespace test;

DataFolder
TwoPersons() {
	DataFolder folder;
	folder.AddPerson("1_Anna", PersonFile(1))
		.AddPerson("2_Bernd", PersonFile(2))
		.SetRelations({"1,2,StrictlyLower"});
	return folder;
}

size_t
Depth(const std::string& aLine) {
	return aLine.find_first_not_of('\t');
}

// docs/requirements/req_0044.md
bool
WorksWithoutTrace() {
	auto folder = TwoPersons();
	ContextGuard context;
	CHECK(folder.Create(&context, nullptr), FT_OK);
	size_t count = 0;
	CHECK(FT_GetPersonCount(context.Get(), &count, nullptr), FT_OK);
	CHECK(count, 2U);
	return true;
}

// docs/requirements/req_0044.md
bool
WorksWithATraceOfCapacityZero() {
	auto folder = TwoPersons();
	ContextGuard context;
	FT_Trace trace{};
	CHECK(folder.Create(&context, &trace), FT_OK);
	CHECK(trace.used, 0U);
	return true;
}

// docs/requirements/req_0044.md
bool
ReportsItsStepsToTheTrace() {
	auto folder = TwoPersons();
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK);
	CHECK(trace.Get().used > 0, true);
	CHECK(trace.Has("STEP", ""), true);
	CHECK(trace.Text().size(), trace.Get().used);
	return true;
}

// docs/decisions/adr_0007.md
bool
EverySuccessfulCallShowsOkOnItsFirstLine() {
	auto folder = TwoPersons();
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK);
	size_t calls = 0;
	for (const auto& line : trace.Lines()) {
		if (Depth(line) == 0) {
			CHECK(line.rfind("[ OK ]\t", 0), 0U);
			++calls;
		}
	}
	CHECK(calls > 0, true);
	return true;
}

// docs/decisions/adr_0007.md
bool
AWarningShowsInEverySubtraceAroundIt() {
	auto folder = TwoPersons();
	folder.AddPerson("2_Bernd", PersonFile(2, R"("death": "1")"));
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	const auto lines = trace.Lines();
	size_t warning = lines.size();
	for (size_t i = 0; i < lines.size() && warning == lines.size(); ++i) {
		if (lines[i].find("WARNING\t", Depth(lines[i])) == Depth(lines[i])
			&& lines[i].find("death") != std::string::npos) {
			warning = i;
		}
	}
	CHECK(warning < lines.size(), true);
	// Every Subtrace that encloses the warning, i.e. every earlier line with a lower depth that
	// opens a Subtrace, shows [WARN].
	size_t depth = Depth(lines[warning]);
	for (size_t i = warning; i-- > 0;) {
		if (Depth(lines[i]) < depth && lines[i].find('[', Depth(lines[i])) == Depth(lines[i])) {
			CHECK(lines[i].substr(Depth(lines[i]), 6), "[WARN]");
			depth = Depth(lines[i]);
		}
	}
	CHECK(depth, 0U);
	return true;
}

// docs/decisions/adr_0007.md
bool
NoSubtraceIsLeftOpenAfterACall() {
	auto folder = TwoPersons();
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK);
	CHECK(trace.Text().find("[....]"), std::string::npos);
	CHECK(trace.Get().depth, 0U);
	return true;
}

// docs/decisions/adr_0007.md
bool
TheCallThatFailedShowsFailOnItsFirstLine() {
	DataFolder folder;
	folder.AddPerson("1", PersonFile(1))
		.AddPerson("2", PersonFile(2))
		.SetRelations({"1,2,StrictlyLower", "2,1,StrictlyLower"});
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_ERROR_CONFLICTING_GENERATIONS);
	std::string lastCall;
	for (const auto& line : trace.Lines()) {
		if (Depth(line) == 0) {
			lastCall = line;
		}
	}
	CHECK(lastCall.rfind("[FAIL]\t", 0), 0U);
	return true;
}

// docs/decisions/adr_0007.md
bool
IndentsWithOneTabPerDepth() {
	auto folder = TwoPersons();
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK);
	const auto lines = trace.Lines();
	CHECK(lines.size() > 1, true);
	size_t previous = 0;
	for (const auto& line : lines) {
		CHECK(line.empty(), false);
		CHECK(line[0] == ' ', false);
		CHECK(Depth(line) <= previous + 1, true);
		previous = Depth(line);
	}
	return true;
}

// docs/decisions/adr_0007.md
bool
CountsTheLinesThatDoNotFit() {
	auto folder = TwoPersons();
	ContextGuard context;
	TraceBuffer trace(64);
	CHECK(Create(folder, context, trace), FT_OK);
	CHECK(trace.Get().droppedLines > 0, true);
	CHECK(trace.Get().used < trace.Get().capacity, true);
	CHECK(trace.Get().buffer[trace.Get().used], '\0');
	return true;
}

} // namespace

bool
TraceSuite() {
	auto result = SUITE("Trace");
	result &= RUN(WorksWithoutTrace);
	result &= RUN(WorksWithATraceOfCapacityZero);
	result &= RUN(ReportsItsStepsToTheTrace);
	result &= RUN(EverySuccessfulCallShowsOkOnItsFirstLine);
	result &= RUN(AWarningShowsInEverySubtraceAroundIt);
	result &= RUN(NoSubtraceIsLeftOpenAfterACall);
	result &= RUN(TheCallThatFailedShowsFailOnItsFirstLine);
	result &= RUN(IndentsWithOneTabPerDepth);
	result &= RUN(CountsTheLinesThatDoNotFit);
	return result;
}
