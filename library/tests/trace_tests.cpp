// Unit tests of the Trace writer: the format of docs/decisions/adr_0018.md, and working without a
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

// docs/decisions/adr_0018.md
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

// docs/decisions/adr_0018.md
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

// docs/decisions/adr_0018.md
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

// docs/decisions/adr_0018.md
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

// docs/decisions/adr_0018.md
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

// docs/decisions/adr_0018.md
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

// Whether a line starts with the given text after its indentation, e.g. "STEP\t" or "[ OK ]".
bool
StartsWith(const std::string& aLine, const std::string& aText) {
	return aLine.compare(Depth(aLine), aText.size(), aText) == 0;
}

// docs/decisions/adr_0018.md
bool
ASuccessfulCallKeepsOnlyItsSummary() {
	auto folder = TwoPersons();
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK);
	// Every call keeps its first line, followed by at most one summary line.
	const auto lines = trace.Lines();
	for (size_t i = 0; i < lines.size(); ++i) {
		CHECK(Depth(lines[i]) <= 1, true);
		if (Depth(lines[i]) == 1) {
			CHECK(StartsWith(lines[i], "STEP\t"), true);
			CHECK(Depth(lines[i - 1]), 0U);
		}
	}
	return true;
}

// docs/decisions/adr_0018.md
bool
AWarningKeepsItsLinesAfterTheSummary() {
	auto folder = TwoPersons();
	folder.AddPerson("2_Bernd", PersonFile(2, R"("death": "1", "birth": "2")"));
	ContextGuard context;
	TraceBuffer trace;
	CHECK(Create(folder, context, trace), FT_OK_WITH_WARNINGS);
	CHECK(trace.HasWarning("death"), true);
	CHECK(trace.HasWarning("birth"), true);
	const auto lines = trace.Lines();
	for (size_t i = 0; i < lines.size(); ++i) {
		// Nested Subtraces that succeeded are gone once the Subtrace around them closed.
		CHECK(Depth(lines[i]) > 0 && StartsWith(lines[i], "[ OK ]"), false);
		// Within a Subtrace, a summary comes first: no step follows a warning or a nested
		// Subtrace at the same depth.
		if (!StartsWith(lines[i], "STEP\t")) {
			continue;
		}
		for (size_t j = i; j > 0 && Depth(lines[j - 1]) >= Depth(lines[i]); --j) {
			CHECK(Depth(lines[j - 1]) == Depth(lines[i]), false);
		}
	}
	return true;
}

// docs/decisions/adr_0018.md
bool
ATooSmallTraceKeepsTheNewestLines() {
	auto fullFolder = TwoPersons();
	ContextGuard fullContext;
	TraceBuffer full;
	CHECK(Create(fullFolder, fullContext, full), FT_OK);
	auto smallFolder = TwoPersons();
	ContextGuard smallContext;
	TraceBuffer small(512);
	CHECK(Create(smallFolder, smallContext, small), FT_OK);
	CHECK(small.Get().droppedLines > 0, true);
	// The first call is dropped, and the last call is kept as a whole.
	CHECK(small.Lines().front() == full.Lines().front(), false);
	const auto fullText = full.Text();
	const auto smallText = small.Text();
	const auto lastCall = fullText.rfind("\n[") + 1;
	CHECK(smallText.size() >= fullText.size() - lastCall, true);
	CHECK(
		smallText.substr(smallText.size() - (fullText.size() - lastCall)),
		fullText.substr(lastCall));
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
	result &= RUN(ASuccessfulCallKeepsOnlyItsSummary);
	result &= RUN(AWarningKeepsItsLinesAfterTheSummary);
	result &= RUN(ATooSmallTraceKeepsTheNewestLines);
	return result;
}
