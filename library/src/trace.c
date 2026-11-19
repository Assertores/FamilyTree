// Trace writer: the only code that touches an FT_Trace. Subtraces are compacted when they close,
// and a full buffer drops its oldest lines (docs/decisions/adr_0018.md).

#include "internal.h"

#include <assert.h>
#include <family_tree/api.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

enum {
	TRACE_MESSAGE_SIZE = 512, // a longer message is cut off
	TRACE_DROP_SHARE = 4,	  // a full buffer drops at least this part of its capacity at once
};

static const char theOpenOutcome[] = "[....]";
static const char theOkOutcome[] = "[ OK ]";
static const char theWarningOutcome[] = "[WARN]";
static const char theFailOutcome[] = "[FAIL]";
static const char theStepLevel[] = "STEP";
static const char theWarningLevel[] = "WARNING";
static const char theErrorLevel[] = "ERROR";
static const size_t theNotFound = (size_t)-1;

// The lines a closing Subtrace removed, for its summary.
typedef struct {
	size_t steps;
	size_t subtraces;
} Removed;

static bool
PrivIsActive(const FT_Trace* aTrace) {
	return aTrace != NULL && aTrace->buffer != NULL && aTrace->capacity > 0;
}

// Formats a message into aOut, of TRACE_MESSAGE_SIZE bytes, on a single line.
static void
PrivFormat(char* aOut, const char* aFormat, va_list aArguments) {
	if (vsnprintf(aOut, TRACE_MESSAGE_SIZE, aFormat, aArguments) < 0) {
		aOut[0] = '\0';
	}
	for (size_t i = 0; i < TRACE_MESSAGE_SIZE && aOut[i] != '\0'; ++i) {
		if (aOut[i] == '\n' || aOut[i] == '\r' || aOut[i] == '\t') {
			aOut[i] = ' ';
		}
	}
}

// The length of a line of tabs, a head, an optional middle and a text, with its newline.
static size_t
PrivLineLength(size_t aIndentation, const char* aHead, const char* aMiddle, const char* aText) {
	const size_t middle = aMiddle != NULL ? strlen(aMiddle) + 1 : 0;
	return aIndentation + strlen(aHead) + 1 + middle + strlen(aText) + 1;
}

// Copies a text without its terminating zero, which the line does not have; returns its length.
// The text never lies in the Trace buffer, so the two do not overlap.
static size_t
PrivCopyField(char* aOut, const char* aText) {
	const size_t length = strlen(aText);
	// A field ends with a tab or a newline, which the caller writes, never with a zero.
	// NOLINTNEXTLINE(bugprone-not-null-terminated-result)
	(void)memcpy(aOut, aText, length);
	return length;
}

static void
PrivWriteLine(
	char* aOut,
	size_t aIndentation,
	const char* aHead,
	const char* aMiddle,
	const char* aText) {
	size_t used = aIndentation;
	(void)memset(aOut, '\t', aIndentation);
	used += PrivCopyField(aOut + used, aHead);
	aOut[used] = '\t';
	++used;
	if (aMiddle != NULL) {
		used += PrivCopyField(aOut + used, aMiddle);
		aOut[used] = '\t';
		++used;
	}
	used += PrivCopyField(aOut + used, aText);
	aOut[used] = '\n';
}

// The index just after the newline of the line that starts at aStart.
static size_t
PrivLineEnd(const FT_Trace* aTrace, size_t aStart) {
	const char* newline = memchr(aTrace->buffer + aStart, '\n', aTrace->used - aStart);
	return newline != NULL ? (size_t)(newline - aTrace->buffer) + 1 : aTrace->used;
}

// Drops whole lines from the beginning, at least aNeeded bytes and a share of the capacity.
static void
PrivDropOldest(FT_Trace* aTrace, size_t aNeeded) {
	const size_t share = aTrace->capacity / TRACE_DROP_SHARE;
	const size_t amount = aNeeded > share ? aNeeded : share;
	size_t cut = 0;
	size_t lines = 0;
	while (cut < aTrace->used && cut < amount) {
		cut = PrivLineEnd(aTrace, cut);
		++lines;
	}
	(void)memmove(aTrace->buffer, aTrace->buffer + cut, aTrace->used - cut);
	aTrace->used -= cut;
	aTrace->buffer[aTrace->used] = '\0';
	aTrace->droppedLines += lines;
}

static void
PrivAppendLine(FT_Trace* aTrace, const char* aHead, const char* aMiddle, const char* aText) {
	const size_t length = PrivLineLength(aTrace->depth, aHead, aMiddle, aText);
	if (length >= aTrace->capacity) {
		++aTrace->droppedLines;
		return;
	}
	if (aTrace->used + length >= aTrace->capacity) {
		PrivDropOldest(aTrace, aTrace->used + length + 1 - aTrace->capacity);
	}
	PrivWriteLine(aTrace->buffer + aTrace->used, aTrace->depth, aHead, aMiddle, aText);
	aTrace->used += length;
	aTrace->buffer[aTrace->used] = '\0';
}

// Appends a line of aHead, aMiddle if it is not NULL, and the formatted message: the level and the
// message of a report, or the outcome, the function and the description of a Subtrace.
static void
PrivReport(
	FT_Trace* aTrace,
	const char* aHead,
	const char* aMiddle,
	const char* aFormat,
	va_list aArguments) {
	char message[TRACE_MESSAGE_SIZE] = {0};
	PrivFormat(message, aFormat, aArguments);
	PrivAppendLine(aTrace, aHead, aMiddle, message);
}

void
TraceOpen(FT_Trace* aTrace, const char* aFunction, const char* aFormat, ...) {
	if (!PrivIsActive(aTrace)) {
		return;
	}
	va_list arguments; // NOLINT(cppcoreguidelines-init-variables): initialized by va_start
	va_start(arguments, aFormat);
	PrivReport(aTrace, theOpenOutcome, aFunction, aFormat, arguments);
	va_end(arguments);
	++aTrace->depth;
}

void
TraceStep(FT_Trace* aTrace, const char* aFormat, ...) {
	if (!PrivIsActive(aTrace)) {
		return;
	}
	va_list arguments; // NOLINT(cppcoreguidelines-init-variables): initialized by va_start
	va_start(arguments, aFormat);
	PrivReport(aTrace, theStepLevel, NULL, aFormat, arguments);
	va_end(arguments);
}

void
TraceWarning(FT_Trace* aTrace, const char* aFormat, ...) {
	if (!PrivIsActive(aTrace)) {
		return;
	}
	va_list arguments; // NOLINT(cppcoreguidelines-init-variables): initialized by va_start
	va_start(arguments, aFormat);
	PrivReport(aTrace, theWarningLevel, NULL, aFormat, arguments);
	va_end(arguments);
}

void
TraceError(FT_Trace* aTrace, const char* aFormat, ...) {
	if (!PrivIsActive(aTrace)) {
		return;
	}
	va_list arguments; // NOLINT(cppcoreguidelines-init-variables): initialized by va_start
	va_start(arguments, aFormat);
	PrivReport(aTrace, theErrorLevel, NULL, aFormat, arguments);
	va_end(arguments);
}

// The number of tabs a line is indented by.
static size_t
PrivIndentation(const char* aLine) {
	return strspn(aLine, "\t");
}

// The first line of the innermost open Subtrace: the last line indented by aIndentation tabs,
// as every line written after it is indented deeper. theNotFound if it was dropped.
static size_t
PrivFindFirstLine(const FT_Trace* aTrace, size_t aIndentation) {
	for (size_t end = aTrace->used, start = 0; end > 0; end = start) {
		for (start = end - 1; start > 0 && aTrace->buffer[start - 1] != '\n'; --start) {
		}
		const char* line = aTrace->buffer + start;
		const size_t indentation = PrivIndentation(line);
		if (indentation < aIndentation) {
			return theNotFound;
		}
		if (indentation == aIndentation) {
			return StartsWith(line + indentation, theOpenOutcome) ? start : theNotFound;
		}
	}
	return theNotFound;
}

// How a Subtrace ended, whichever type of result it was closed with.
typedef enum {
	OUTCOME_OK,
	OUTCOME_WARNING,
	OUTCOME_FAIL,
} Outcome;

static Outcome
PrivOutcome(FT_Result aResult) {
	switch (aResult) {
	case FT_OK:
		return OUTCOME_OK;
	case FT_OK_WITH_WARNINGS:
		return OUTCOME_WARNING;
	case FT_ERROR_INVALID_ARGUMENT:
	case FT_ERROR_UNKNOWN_PERSON:
	case FT_ERROR_UNKNOWN_FAMILY:
	case FT_ERROR_BUFFER_TOO_SMALL:
	case FT_ERROR_OUT_OF_MEMORY:
	case FT_ERROR_UNEXPECTED_ANSWER:
	case FT_ERROR_PLATFORM:
	case FT_ERROR_NOT_SUPPORTED:
	case FT_ERROR_CONFLICTING_GENERATIONS:
		return OUTCOME_FAIL;
	}
	return OUTCOME_FAIL;
}

static Outcome
PrivResultOutcome(Result aResult) {
	switch (aResult) {
	case RESULT_OK:
		return OUTCOME_OK;
	case RESULT_OK_WITH_WARNINGS:
		return OUTCOME_WARNING;
	case RESULT_OUT_OF_MEMORY:
	case RESULT_UNREADABLE:
		return OUTCOME_FAIL;
	}
	return OUTCOME_FAIL;
}

static const char*
PrivOutcomeText(Outcome aOutcome) {
	switch (aOutcome) {
	case OUTCOME_OK:
		return theOkOutcome;
	case OUTCOME_WARNING:
		return theWarningOutcome;
	case OUTCOME_FAIL:
		return theFailOutcome;
	}
	return theFailOutcome;
}

// Whether a line of a closing Subtrace stays. aIndentation is the indentation of its own lines;
// a nested Subtrace stays with all its lines if it did not succeed and warnings are kept.
static bool
PrivKeepLine(
	const char* aLine,
	size_t aIndentation,
	bool aKeepWarnings,
	bool* aInKeptSubtrace,
	Removed* aRemoved) {
	const size_t indentation = PrivIndentation(aLine);
	if (indentation != aIndentation) {
		return indentation < aIndentation || *aInKeptSubtrace;
	}
	if (aLine[indentation] == '[') {
		*aInKeptSubtrace = aKeepWarnings && !StartsWith(aLine + indentation, theOkOutcome);
		if (!*aInKeptSubtrace) {
			++aRemoved->subtraces;
		}
		return *aInKeptSubtrace;
	}
	*aInKeptSubtrace = false;
	if (StartsWith(aLine + indentation, theStepLevel)) {
		++aRemoved->steps;
		return false;
	}
	return aKeepWarnings;
}

static void
PrivDescribeRemoved(char* aOut, Removed aRemoved) {
	const char* steps = aRemoved.steps == 1 ? "step" : "steps";
	const char* subtraces = aRemoved.subtraces == 1 ? "Subtrace" : "Subtraces";
	int written = 0;
	if (aRemoved.subtraces == 0) {
		written = snprintf(aOut, TRACE_MESSAGE_SIZE, "%zu %s", aRemoved.steps, steps);
	} else if (aRemoved.steps == 0) {
		written = snprintf(aOut, TRACE_MESSAGE_SIZE, "%zu %s", aRemoved.subtraces, subtraces);
	} else {
		written = snprintf(
			aOut,
			TRACE_MESSAGE_SIZE,
			"%zu %s, %zu %s",
			aRemoved.steps,
			steps,
			aRemoved.subtraces,
			subtraces);
	}
	if (written < 0) {
		aOut[0] = '\0';
	}
}

// Inserts the summary line at aAt, if there is room; otherwise it counts as dropped.
static void
PrivInsertSummary(FT_Trace* aTrace, size_t aAt, size_t aIndentation, Removed aRemoved) {
	if (aRemoved.steps == 0 && aRemoved.subtraces == 0) {
		return;
	}
	char summary[TRACE_MESSAGE_SIZE] = {0};
	PrivDescribeRemoved(summary, aRemoved);
	const size_t length = PrivLineLength(aIndentation, theStepLevel, NULL, summary);
	if (aTrace->used + length >= aTrace->capacity) {
		++aTrace->droppedLines;
		return;
	}
	char* position = aTrace->buffer + aAt;
	(void)memmove(position + length, position, aTrace->used - aAt);
	PrivWriteLine(position, aIndentation, theStepLevel, NULL, summary);
	aTrace->used += length;
	aTrace->buffer[aTrace->used] = '\0';
}

// Removes the steps and the successful nested Subtraces after the first line at aFirst, and, if
// aKeepWarnings is false, everything else as well; then inserts the summary.
static void
PrivCompact(FT_Trace* aTrace, size_t aFirst, bool aKeepWarnings) {
	const size_t start = PrivLineEnd(aTrace, aFirst);
	const size_t indentation = aTrace->depth + 1;
	size_t write = start;
	bool inKeptSubtrace = false;
	Removed removed = {0, 0};
	for (size_t read = start, end = 0; read < aTrace->used; read = end) {
		end = PrivLineEnd(aTrace, read);
		if (PrivKeepLine(
				aTrace->buffer + read,
				indentation,
				aKeepWarnings,
				&inKeptSubtrace,
				&removed)) {
			(void)memmove(aTrace->buffer + write, aTrace->buffer + read, end - read);
			write += end - read;
		}
	}
	aTrace->used = write;
	aTrace->buffer[write] = '\0';
	PrivInsertSummary(aTrace, start, indentation, removed);
}

// Writes the outcome into the first line of the innermost Subtrace, and compacts it unless it
// failed, as a failed Subtrace keeps every line.
static void
PrivCloseWith(FT_Trace* aTrace, Outcome aOutcome) {
	if (!PrivIsActive(aTrace) || aTrace->depth == 0) {
		return;
	}
	--aTrace->depth;
	const size_t first = PrivFindFirstLine(aTrace, aTrace->depth);
	if (first == theNotFound) {
		return;
	}
	(void)PrivCopyField(aTrace->buffer + first + aTrace->depth, PrivOutcomeText(aOutcome));
	if (aOutcome == OUTCOME_FAIL) {
		return;
	}
	PrivCompact(aTrace, first, aOutcome == OUTCOME_WARNING);
}

FT_Result
TraceClose(FT_Trace* aTrace, FT_Result aResult) {
	PrivCloseWith(aTrace, PrivOutcome(aResult));
	return aResult;
}

Result
TraceCloseResult(FT_Trace* aTrace, Result aResult) {
	PrivCloseWith(aTrace, PrivResultOutcome(aResult));
	return aResult;
}

FT_Result
TraceFinish(FT_Trace* aTrace, FT_Result aResult) {
	// The linear buffer is plain text at all times; a ring buffer would be turned back into plain
	// text here (docs/decisions/adr_0018.md#a-ring-buffer-instead-of-dropping-a-quarter).
	if (PrivIsActive(aTrace)) {
		assert(aTrace->used < aTrace->capacity && aTrace->buffer[aTrace->used] == '\0');
	}
	return aResult;
}

FT_Result
TraceEndCall(FT_Trace* aTrace, FT_Result aResult) {
	return TraceFinish(aTrace, TraceClose(aTrace, aResult));
}
