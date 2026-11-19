// CSV parser for the Relations File: the text is split into lines, and each line into fields, in
// place (docs/requirements/req_0006.md, docs/requirements/req_0007.md, docs/decisions/adr_0019.md).

#include "internal.h"

#include <family_tree/api.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

enum {
	CSV_MAX_FIELDS = 6,
	CSV_MANDATORY_FIELDS = 3,
	CSV_FIELD_RELATIONSHIP = 3,
	CSV_FIELD_START_DATE = 4,
	CSV_FIELD_END_DATE = 5,
	CSV_HEADER_LINES = 2, // the first line holds the Format Version, the second is ignored
};

// The Format Version this parser reads (docs/decisions/adr_0016.md).
static const char theRelationsFileVersion[] = "227E82BA-6603-426C-9C95-5BC86A10A642";

typedef struct {
	char* fields[CSV_MAX_FIELDS];
	size_t count;
	bool tooMany;
} Row;

// What the warnings about the file need.
typedef struct {
	const char* file; // as warnings name it
	FT_Trace* trace;
	bool warned;
} CsvReport;

// The lines of a text, each zero terminated.
typedef struct {
	char** items;
	size_t count;
} Lines;

// Splits a text into lines in place: each "\n" becomes a zero, and so does a "\r" before it, as a
// line break always ends a line, also within quotation marks. The array goes into aArena; false if
// memory ran out.
static bool
PrivSplitLines(char* aText, Arena* aArena, Lines* aOut) {
	const size_t length = strlen(aText);
	size_t count = 1;
	for (size_t i = 0; i < length; ++i) {
		count += aText[i] == '\n' ? 1 : 0;
	}
	if (count > SIZE_MAX / sizeof(char*)) {
		return false;
	}
	char** items = (char**)ArenaAllocate(aArena, count * sizeof(char*));
	if (items == NULL) {
		return false;
	}
	items[0] = aText;
	for (size_t i = 0, line = 0; i < length; ++i) {
		if (aText[i] == '\n') {
			aText[i] = '\0';
			++line;
			items[line] = aText + i + 1;
		}
		if (aText[i] == '\r' && (aText[i + 1] == '\n' || aText[i + 1] == '\0')) {
			aText[i] = '\0';
		}
	}
	aOut->items = items;
	aOut->count = count;
	return true;
}

// Unquotes the field at aPosition in place; false if its quotation marks are not closed, or text
// follows the closing one.
static bool
PrivSplitQuoted(char* aLine, size_t aLength, size_t* aPosition, char** aOutField) {
	size_t read = *aPosition + 1;
	size_t write = *aPosition;
	*aOutField = aLine + write;
	while (read < aLength) {
		if (aLine[read] != '"') {
			aLine[write] = aLine[read];
			++write;
			++read;
			continue;
		}
		if (aLine[read + 1] == '"') {
			aLine[write] = '"'; // a doubled quotation mark
			++write;
			read += 2;
			continue;
		}
		// The closing quotation mark, which only a comma or the end of the line may follow.
		const char after = aLine[read + 1];
		aLine[write] = '\0';
		*aPosition = read + 2;
		return after == ',' || after == '\0';
	}
	return false;
}

// Splits off the field at aPosition, and moves aPosition to the next field, or past the end.
static bool
PrivSplitField(char* aLine, size_t aLength, size_t* aPosition, char** aOutField) {
	char* start = aLine + *aPosition;
	if (start[0] == '"') {
		return PrivSplitQuoted(aLine, aLength, aPosition, aOutField);
	}
	*aOutField = start;
	char* comma = strchr(start, ',');
	if (comma == NULL) {
		*aPosition = aLength + 1; // the last field
		return true;
	}
	*comma = '\0';
	*aPosition = (size_t)(comma - aLine) + 1;
	return true;
}

static bool
PrivSplitRow(char* aLine, Row* aOut) {
	const size_t length = strlen(aLine);
	size_t position = 0;
	aOut->count = 0;
	aOut->tooMany = false;
	// Only a bound: each pass splits off one field, and a line of n characters has at most n + 1.
	for (size_t pass = 0; pass <= length; ++pass) {
		char* field = NULL;
		if (!PrivSplitField(aLine, length, &position, &field)) {
			return false;
		}
		if (aOut->count < CSV_MAX_FIELDS) {
			aOut->fields[aOut->count] = field;
			++aOut->count;
		} else {
			aOut->tooMany = true;
		}
		if (position > length) {
			return true;
		}
	}
	return true;
}

static FT_RelationType
PrivRelationType(CsvReport* aReport, const char* aType, size_t aLine) {
	if (strcmp(aType, "Unrestricted") == 0) {
		return FT_RELATION_UNRESTRICTED;
	}
	if (strcmp(aType, "StrictlyLower") == 0) {
		return FT_RELATION_STRICTLY_LOWER;
	}
	if (strcmp(aType, "StrictlyHigher") == 0) {
		return FT_RELATION_STRICTLY_HIGHER;
	}
	if (aType[0] == '\0') {
		TraceWarning(
			aReport->trace,
			"%s: line %zu: empty Relation Type, read as Unrestricted",
			aReport->file,
			aLine);
	} else {
		TraceWarning(
			aReport->trace,
			"%s: line %zu: unknown Relation Type \"%s\", read as Unrestricted",
			aReport->file,
			aLine,
			aType);
	}
	aReport->warned = true;
	return FT_RELATION_UNRESTRICTED;
}

static const char*
PrivOptional(const Row* aRow, size_t aField) {
	if (aField >= aRow->count || aRow->fields[aField][0] == '\0') {
		return NULL;
	}
	return aRow->fields[aField];
}

static void
PrivSkipRow(CsvReport* aReport, size_t aLine, const char* aReason) {
	TraceWarning(aReport->trace, "%s: line %zu %s, skipped", aReport->file, aLine, aReason);
	aReport->warned = true;
}

// Splits a line into a row; false if the row is skipped, which was reported.
static bool
PrivReadRow(CsvReport* aReport, char* aLine, size_t aLineNumber, Row* aOut) {
	if (!IsValidUtf8(aLine)) {
		PrivSkipRow(aReport, aLineNumber, "is not valid UTF-8");
		return false;
	}
	if (!PrivSplitRow(aLine, aOut)) {
		PrivSkipRow(aReport, aLineNumber, "has a quotation mark that is not closed");
		return false;
	}
	if (aOut->count < CSV_MANDATORY_FIELDS) {
		PrivSkipRow(aReport, aLineNumber, "has fewer than three columns");
		return false;
	}
	if (aOut->tooMany) {
		TraceWarning(
			aReport->trace,
			"%s: line %zu has more than six columns, the rest is ignored",
			aReport->file,
			aLineNumber);
		aReport->warned = true;
	}
	return true;
}

// Adds the Relation of a line to aRelations; empty lines and skipped rows add nothing.
static void
PrivReadRelation(
	CsvReport* aReport,
	char* aLine,
	size_t aLineNumber,
	FT_Relation* aRelations,
	size_t* aCount) {
	Row row;
	(void)memset(&row, 0, sizeof(row));
	if (aLine[0] == '\0') {
		return; // an empty line
	}
	if (!PrivReadRow(aReport, aLine, aLineNumber, &row)) {
		return; // skipped, which was reported
	}
	FT_Relation relation;
	(void)memset(&relation, 0, sizeof(relation));
	if (!ParseInteger(row.fields[0], strlen(row.fields[0]), &relation.first)) {
		PrivSkipRow(aReport, aLineNumber, "has a Person Id that is not an integer");
		return;
	}
	if (!ParseInteger(row.fields[1], strlen(row.fields[1]), &relation.second)) {
		PrivSkipRow(aReport, aLineNumber, "has a Person Id that is not an integer");
		return;
	}
	relation.type = PrivRelationType(aReport, row.fields[2], aLineNumber);
	relation.relationship = PrivOptional(&row, CSV_FIELD_RELATIONSHIP);
	relation.startDate = PrivOptional(&row, CSV_FIELD_START_DATE);
	relation.endDate = PrivOptional(&row, CSV_FIELD_END_DATE);
	aRelations[*aCount] = relation;
	++*aCount;
}

// The Format Version from the first line, "Version" and the UUID; NULL if there is none.
static const char*
PrivReadFormatVersion(char* aLine) {
	Row row;
	(void)memset(&row, 0, sizeof(row));
	if (!PrivSplitRow(aLine, &row)) {
		return NULL;
	}
	if (row.count < 2 || strcmp(row.fields[0], "Version") != 0) {
		return NULL;
	}
	return row.fields[1];
}

// Whether the first line holds a Format Version the Library supports; the rest of a file with
// another Format Version may mean something else (docs/decisions/adr_0020.md).
static bool
PrivHasSupportedVersion(CsvReport* aReport, char* aFirstLine) {
	const char* version = PrivReadFormatVersion(aFirstLine);
	if (version == NULL) {
		TraceError(aReport->trace, "%s: it has no Format Version in its first line", aReport->file);
		return false;
	}
	if (!EqualsIgnoringCase(version, theRelationsFileVersion)) {
		TraceError(
			aReport->trace,
			"%s: the Format Version %s is not supported",
			aReport->file,
			version);
		return false;
	}
	return true;
}

Result
ParseRelationsFile(
	char* aText,
	const char* aFile,
	Arena* aScratch,
	Arena* aArena,
	FT_RelationList* aOutList,
	FT_Trace* aTrace) {
	TraceOpen(aTrace, "ParseRelationsFile", "%s", aFile);
	(void)memset(aOutList, 0, sizeof(*aOutList));
	if (HasOtherByteOrderMark(aText)) {
		TraceError(aTrace, "%s: the file is UTF-16 or UTF-32, save it as UTF-8", aFile);
		return TraceCloseResult(aTrace, RESULT_UNREADABLE);
	}
	TraceStep(aTrace, "splitting the text into lines");
	Lines lines = {NULL, 0};
	if (!PrivSplitLines(aText + ByteOrderMarkLength(aText), aScratch, &lines)) {
		TraceError(aTrace, "%s: memory ran out while parsing", aFile);
		return TraceCloseResult(aTrace, RESULT_OUT_OF_MEMORY);
	}
	CsvReport report = {aFile, aTrace, false};
	if (!PrivHasSupportedVersion(&report, lines.items[0])) {
		return TraceCloseResult(aTrace, RESULT_UNREADABLE); // reported at the Format Version
	}
	const size_t rows = lines.count > CSV_HEADER_LINES ? lines.count - CSV_HEADER_LINES : 0;
	TraceStep(aTrace, "reading %zu rows", rows);
	FT_Relation* relations = ArenaAllocate(aArena, rows * sizeof(FT_Relation));
	if (relations == NULL) {
		TraceError(aTrace, "%s: memory ran out while parsing", aFile);
		return TraceCloseResult(aTrace, RESULT_OUT_OF_MEMORY);
	}
	size_t count = 0;
	for (size_t i = CSV_HEADER_LINES; i < lines.count; ++i) {
		PrivReadRelation(&report, lines.items[i], i + 1, relations, &count);
	}
	aOutList->items = relations;
	aOutList->count = count;
	return TraceCloseResult(aTrace, report.warned ? RESULT_OK_WITH_WARNINGS : RESULT_OK);
}
