// JSON parser for Person Files, in three parts (docs/decisions/adr_0021.md):
//
// 1. Tokenizer   finds the next token of JSON. Keeps no state: all it gets is a position in the
//                text, and all it returns is the token and where the next one starts.
// 2. Driver      one loop over the tokens. Owns the stack of frames and checks the grammar of
//                JSON: where a key, a colon, a comma, a value or a closing bracket may stand.
//                Pushes the frames the frames return, and pops a frame when it is complete.
//                Knows nothing about Persons.
// 3. Frames      one kind for each part of the Person File. Know what their part means: which
//                frame reads an object or the value of a key, where a value goes, what a wrong
//                type is. Know nothing about commas, colons or brackets.
//
// The driver tells the frame on top what arrives: OnObject, OnKey, OnArray, OnValue, OnClose.
// Each is a switch over the kind of the frame: the set of kinds is closed and known when
// compiling, so no function pointers are needed (docs/decisions/adr_0011.md), and -Wswitch warns
// when a kind is added but not handled everywhere. Strings are decoded in place
// (docs/decisions/adr_0019.md).

#include "internal.h"

#include <assert.h>
#include <family_tree/api.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

enum {
	// Frames on the stack; nesting deeper than this is malformed. A Person File needs 5: the
	// document,
	// the Person, "PlaceOfResidence", a Place of Residence and its key. The rest is for the values
	// of unknown keys, e.g. objects 6 levels deep within one, as each level is the frame of an
	// object
	// and of a key; every open frame holds an open Subtrace, which the Trace cannot compact.
	JSON_MAX_DEPTH = 16,
	JSON_HEX_DIGITS = 4,
	JSON_HEX_BASE = 16,
	JSON_HEX_LETTER_OFFSET = 10,
	JSON_UNICODE_ESCAPE_LENGTH = 6, // \uXXXX
	JSON_FIRST_PRINTABLE = 0x20,
};

// The Format Version this parser reads (docs/decisions/adr_0016.md).
static const char thePersonFileVersion[] = "1F10B769-08DE-442F-85BA-737B7F2B4803";

// Code points and their UTF-8 encoding (RFC 3629, RFC 8259).
enum {
	UNICODE_HIGH_SURROGATE_FIRST = 0xD800,
	UNICODE_HIGH_SURROGATE_LAST = 0xDBFF,
	UNICODE_LOW_SURROGATE_FIRST = 0xDC00,
	UNICODE_LOW_SURROGATE_LAST = 0xDFFF,
	UNICODE_SUPPLEMENTARY_FIRST = 0x10000,
	UNICODE_SURROGATE_BITS = 10,
	UTF8_ONE_BYTE_LIMIT = 0x80,
	UTF8_TWO_BYTES_LIMIT = 0x800,
	UTF8_CONTINUATION = 0x80,
	UTF8_CONTINUATION_BITS = 6,
	UTF8_CONTINUATION_MASK = 0x3F,
	UTF8_TWO_BYTES_LEAD = 0xC0,
	UTF8_THREE_BYTES_LEAD = 0xE0,
	UTF8_FOUR_BYTES_LEAD = 0xF0,
};

// =================================================================================================
// 1. Tokenizer
// =================================================================================================

typedef enum {
	TOKEN_OBJECT_BEGIN,
	TOKEN_OBJECT_END,
	TOKEN_ARRAY_BEGIN,
	TOKEN_ARRAY_END,
	TOKEN_COLON,
	TOKEN_COMMA,
	TOKEN_STRING,
	TOKEN_NUMBER,
	TOKEN_LITERAL, // true, false, null
	TOKEN_END,	   // the terminating zero
	TOKEN_MALFORMED,
} TokenType;

typedef struct {
	TokenType type;
	char* start;   // where the token starts, after whitespace, to name the position in warnings
	char* text;	   // STRING: decoded in place, zero terminated; NUMBER, LITERAL: where it starts
	size_t length; // NUMBER, LITERAL
	bool valid;	   // STRING: valid UTF-8, without a zero character
	char* next;	   // where the next token starts; never before start, so every pass moves forward
} Token;

static Token
PrivToken(TokenType aType, char* aStart, char* aNext) {
	const Token token = {aType, aStart, NULL, 0, true, aNext};
	return token;
}

static Token
PrivMalformed(char* aStart) {
	return PrivToken(TOKEN_MALFORMED, aStart, aStart);
}

// The first character that is no whitespace; the terminating zero ends the loop at the latest.
static char*
PrivSkipWhitespace(char* aText) {
	char* position = aText;
	for (; *position == ' ' || *position == '\t' || *position == '\n' || *position == '\r';
		 ++position) {
	}
	return position;
}

static bool
PrivReadHex(const char* aText, unsigned long* aOut) {
	unsigned long value = 0;
	for (size_t i = 0; i < JSON_HEX_DIGITS; ++i) {
		const char digit = aText[i];
		unsigned long digitValue = 0;
		if (digit >= '0' && digit <= '9') {
			digitValue = (unsigned long)(digit - '0');
		} else if (digit >= 'a' && digit <= 'f') {
			digitValue = (unsigned long)(digit - 'a') + JSON_HEX_LETTER_OFFSET;
		} else if (digit >= 'A' && digit <= 'F') {
			digitValue = (unsigned long)(digit - 'A') + JSON_HEX_LETTER_OFFSET;
		} else {
			return false; // also at the terminating zero, so nothing after it is read
		}
		value = (value * JSON_HEX_BASE) + digitValue;
	}
	*aOut = value;
	return true;
}

// Writes a code point as UTF-8, and returns the number of bytes.
static size_t
PrivEncodeUtf8(unsigned long aCode, char* aOut) {
	if (aCode < UTF8_ONE_BYTE_LIMIT) {
		aOut[0] = (char)aCode;
		return 1;
	}
	size_t length = 4;
	unsigned long lead = UTF8_FOUR_BYTES_LEAD;
	if (aCode < UTF8_TWO_BYTES_LIMIT) {
		length = 2;
		lead = UTF8_TWO_BYTES_LEAD;
	} else if (aCode < UNICODE_SUPPLEMENTARY_FIRST) {
		length = 3;
		lead = UTF8_THREE_BYTES_LEAD;
	}
	unsigned long rest = aCode;
	for (size_t i = length - 1; i > 0; --i) {
		aOut[i] = (char)((unsigned long)UTF8_CONTINUATION
						 | (rest & (unsigned long)UTF8_CONTINUATION_MASK));
		rest >>= (unsigned)UTF8_CONTINUATION_BITS;
	}
	aOut[0] = (char)(lead | rest);
	return length;
}

// A string being decoded in place: read goes through its encoded form, write writes the decoded
// one, and is never ahead of read, as no decoded character is longer than its source.
typedef struct {
	char* read;
	char* write;
	bool valid;
} Decoding;

// The \uXXXX of a low surrogate at aText, as the second half of a surrogate pair; false if there is
// none.
static bool
PrivReadLowSurrogate(const char* aText, unsigned long* aOut) {
	if (aText[0] != '\\' || aText[1] != 'u') {
		return false;
	}
	unsigned long low = 0;
	if (!PrivReadHex(aText + 2, &low)) {
		return false;
	}
	if (low < UNICODE_LOW_SURROGATE_FIRST || low > UNICODE_LOW_SURROGATE_LAST) {
		return false;
	}
	*aOut = low;
	return true;
}

// Decodes \uXXXX, and a surrogate pair of two of them. A lone surrogate and a zero character make
// the string invalid, but not the file.
static bool
PrivDecodeUnicode(Decoding* aDecoding) {
	unsigned long code = 0;
	if (!PrivReadHex(aDecoding->read + 2, &code)) {
		return false;
	}
	aDecoding->read += JSON_UNICODE_ESCAPE_LENGTH;
	if (code == 0 || (code >= UNICODE_LOW_SURROGATE_FIRST && code <= UNICODE_LOW_SURROGATE_LAST)) {
		aDecoding->valid = false; // a zero character, or a low surrogate without its high one
		return true;
	}
	if (code >= UNICODE_HIGH_SURROGATE_FIRST && code <= UNICODE_HIGH_SURROGATE_LAST) {
		unsigned long low = 0;
		if (!PrivReadLowSurrogate(aDecoding->read, &low)) {
			aDecoding->valid = false;
			return true;
		}
		aDecoding->read += JSON_UNICODE_ESCAPE_LENGTH;
		code = UNICODE_SUPPLEMENTARY_FIRST
			   + ((code - UNICODE_HIGH_SURROGATE_FIRST) << (unsigned)UNICODE_SURROGATE_BITS)
			   + (low - UNICODE_LOW_SURROGATE_FIRST);
	}
	aDecoding->write += PrivEncodeUtf8(code, aDecoding->write);
	return true;
}

static bool
PrivDecodeEscape(Decoding* aDecoding) {
	const char kind = aDecoding->read[1];
	char decoded = kind;
	switch (kind) {
	case '"':
	case '\\':
	case '/':
		break;
	case 'b':
		decoded = '\b';
		break;
	case 'f':
		decoded = '\f';
		break;
	case 'n':
		decoded = '\n';
		break;
	case 'r':
		decoded = '\r';
		break;
	case 't':
		decoded = '\t';
		break;
	case 'u':
		return PrivDecodeUnicode(aDecoding);
	default:
		return false;
	}
	*aDecoding->write = decoded;
	++aDecoding->write;
	aDecoding->read += 2;
	return true;
}

// Copies one character, or decodes one escape sequence. The terminating zero is a control
// character, so a string that is not closed is malformed.
static bool
PrivCopyCharacter(Decoding* aDecoding) {
	const unsigned char current = (unsigned char)*aDecoding->read;
	if (current == '\\') {
		return PrivDecodeEscape(aDecoding);
	}
	if (current < JSON_FIRST_PRINTABLE) {
		return false;
	}
	size_t length = Utf8SequenceLength(aDecoding->read);
	if (length == 0) {
		aDecoding->valid = false;
		length = 1;
	}
	for (size_t i = 0; i < length; ++i) {
		*aDecoding->write = *aDecoding->read;
		++aDecoding->write;
		++aDecoding->read;
	}
	return true;
}

// A string whose quotation mark is at aStart, decoded in place. The decoded string is never longer
// than its encoded form, so the text after it stays untouched.
static Token
PrivReadString(char* aStart) {
	Decoding decoding = {aStart + 1, aStart + 1, true};
	// Ends at the closing quotation mark, or at the terminating zero, which PrivCopyCharacter
	// refuses; each pass consumes at least one character.
	while (*decoding.read != '"') {
		if (!PrivCopyCharacter(&decoding)) {
			return PrivMalformed(aStart);
		}
	}
	Token token = PrivToken(TOKEN_STRING, aStart, decoding.read + 1);
	*decoding.write = '\0';
	token.text = aStart + 1;
	token.valid = decoding.valid;
	return token;
}

static bool
PrivIsDigit(char aCharacter) {
	return aCharacter >= '0' && aCharacter <= '9';
}

// The first character after the digits at aText; the terminating zero ends the loop at the latest.
static char*
PrivSkipDigits(char* aText) {
	char* position = aText;
	for (; PrivIsDigit(*position); ++position) {
	}
	return position;
}

// A number as RFC 8259 defines it; the terminating zero stops every check.
static Token
PrivReadNumber(char* aStart) {
	char* end = aStart;
	if (*end == '-') {
		++end;
	}
	if (!PrivIsDigit(*end)) {
		return PrivMalformed(aStart);
	}
	end = *end == '0' ? end + 1 : PrivSkipDigits(end);
	if (*end == '.') {
		if (!PrivIsDigit(end[1])) {
			return PrivMalformed(aStart);
		}
		end = PrivSkipDigits(end + 1);
	}
	if (*end == 'e' || *end == 'E') {
		++end;
		if (*end == '+' || *end == '-') {
			++end;
		}
		if (!PrivIsDigit(*end)) {
			return PrivMalformed(aStart);
		}
		end = PrivSkipDigits(end);
	}
	Token token = PrivToken(TOKEN_NUMBER, aStart, end);
	token.text = aStart;
	token.length = (size_t)(end - aStart);
	return token;
}

static Token
PrivReadLiteral(char* aStart) {
	static const char* const theLiterals[] = {"true", "false", "null"};
	for (size_t i = 0; i < sizeof(theLiterals) / sizeof(theLiterals[0]); ++i) {
		const size_t length = strlen(theLiterals[i]);
		if (strncmp(aStart, theLiterals[i], length) == 0) {
			Token token = PrivToken(TOKEN_LITERAL, aStart, aStart + length);
			token.text = aStart;
			token.length = length;
			return token;
		}
	}
	return PrivMalformed(aStart);
}

// The token at aText, after whitespace. No bracket stack: whether brackets match is the driver's
// job.
static Token
PrivNextToken(char* aText) {
	char* const start = PrivSkipWhitespace(aText);
	switch (*start) {
	case '{':
		return PrivToken(TOKEN_OBJECT_BEGIN, start, start + 1);
	case '}':
		return PrivToken(TOKEN_OBJECT_END, start, start + 1);
	case '[':
		return PrivToken(TOKEN_ARRAY_BEGIN, start, start + 1);
	case ']':
		return PrivToken(TOKEN_ARRAY_END, start, start + 1);
	case ':':
		return PrivToken(TOKEN_COLON, start, start + 1);
	case ',':
		return PrivToken(TOKEN_COMMA, start, start + 1);
	case '"':
		return PrivReadString(start);
	case 't':
	case 'f':
	case 'n':
		return PrivReadLiteral(start);
	case '\0':
		return PrivToken(TOKEN_END, start, start);
	default:
		return PrivReadNumber(start); // also anything else, which it finds malformed
	}
}

// =================================================================================================
// 3. Frames (before the driver, which calls them)
// =================================================================================================

typedef enum {
	FRAME_DOCUMENT,	  // the whole text, whose value must be the object of the Person
	FRAME_PERSON,	  // the object of the Person: its entries are keys
	FRAME_RESIDENCE,  // the object of one Place of Residence: its entries are keys
	FRAME_VERSION,	  // the value of "Version"; reading stops at a Format Version not supported
	FRAME_ID,		  // the value of "Id"
	FRAME_TEXT,		  // the value of a key that holds a text, e.g. "Title"
	FRAME_TEXT_LIST,  // the value of a key that holds a list of texts, e.g. "FirstNames"
	FRAME_RESIDENCES, // the value of "PlaceOfResidence"
	FRAME_SKIP,		  // a value nobody reads: of an unknown key, a second equal key, a wrong type
} FrameKind;

// Where a frame is in the grammar of JSON, see the state diagram of the ADR. Only the driver reads
// and writes it. An entry is a key in an object, and a value in an array.
typedef enum {
	EXPECT_COLON,		 // pushed for a key, waits for the colon
	EXPECT_VALUE,		 // waits for its one value
	EXPECT_WAITING,		 // its value is an object, whose frame is on top of it
	EXPECT_ENTRY_OR_END, // after "{", or after "[" of its value
	EXPECT_ENTRY,		 // after ","
	EXPECT_COMMA_OR_END, // after an entry
} Expect;

// The values of a list while it is read, newest first, in the scratch Arena.
typedef struct ListNode ListNode;
struct ListNode {
	const char* text;
	FT_Residence residence;
	ListNode* next;
};

// Where a list of texts goes: the items and count of an FT_TextList, or the Pictures of the
// record, which are no FT_TextList.
typedef struct {
	const char* const** items;
	size_t* count;
} ListTarget;

typedef struct {
	FrameKind kind;
	const char* key; // the key whose value this is, for warnings
	union {
		const char** text;
		FT_PersonId* id;
		ListTarget list;
		FT_ResidenceList* residences;
		FT_Residence* residence;
	} target;		  // where the values go
	ListNode* values; // FRAME_TEXT_LIST, FRAME_RESIDENCES: the items so far
	size_t count;	  // of values
	bool array;		  // FRAME_TEXT_LIST, FRAME_RESIDENCES: OnArray arrived, so the list is present
	bool wrongItem;	  // FRAME_TEXT_LIST, FRAME_RESIDENCES: an item of the wrong type skips it all
	unsigned long seen; // FRAME_PERSON, FRAME_RESIDENCE: one bit per key of its object, the first
						// one counts
	// Only for the driver:
	Expect expect;
	bool isArray; // its entries are the values of an array, closed by "]"; otherwise keys, "}"
	size_t warningsBefore; // the warnings written before it was pushed, to tell its outcome
} Frame;

// Everything a call of ParsePersonFile keeps; nothing is static.
typedef struct {
	Frame frames[JSON_MAX_DEPTH];
	size_t depth; // the number of frames on the stack
	FT_PersonRecord* record;
	const char* file; // as warnings name it
	Arena* scratch;	  // for the values of a list while it is read
	Arena* arena;	  // for the arrays of the record
	FT_Trace* trace;
	size_t warnings; // written so far
	bool outOfMemory;
	bool unsupported; // stopped at a Format Version that is not supported
	bool hasVersion;  // a supported Format Version was read
	bool hasId;
} Reader;

// The keys of the Person File (docs/requirements/req_0003.md).
typedef enum {
	PERSON_KEY_VERSION,
	PERSON_KEY_ID,
	PERSON_KEY_TITLE,
	PERSON_KEY_FIRST_NAMES,
	PERSON_KEY_TITLE_OF_NOBILITY,
	PERSON_KEY_LAST_NAMES,
	PERSON_KEY_GENDER,
	PERSON_KEY_DATE_OF_BIRTH,
	PERSON_KEY_PLACE_OF_BIRTH,
	PERSON_KEY_DATE_OF_DEATH,
	PERSON_KEY_PLACE_OF_DEATH,
	PERSON_KEY_PROFESSION,
	PERSON_KEY_PLACE_OF_RESIDENCE,
	PERSON_KEY_REMARKS,
	PERSON_KEY_IMAGES,
	PERSON_KEY_AUDIO,
} PersonKey;

// The keys of a Place of Residence, which are not keys of the Person File, nor the other way round.
typedef enum {
	RESIDENCE_KEY_NAME,
	RESIDENCE_KEY_START_DATE,
	RESIDENCE_KEY_END_DATE,
} ResidenceKey;

typedef struct {
	const char* name;
	FrameKind frame; // the kind of frame that reads its value
} KeyInfo;

// In the order of PersonKey.
static const KeyInfo thePersonKeys[] = {
	{"Version", FRAME_VERSION},
	{"Id", FRAME_ID},
	{"Title", FRAME_TEXT},
	{"FirstNames", FRAME_TEXT_LIST},
	{"TitleOfNobility", FRAME_TEXT},
	{"LastNames", FRAME_TEXT_LIST},
	{"Gender", FRAME_TEXT},
	{"DateOfBirth", FRAME_TEXT},
	{"PlaceOfBirth", FRAME_TEXT},
	{"DateOfDeath", FRAME_TEXT},
	{"PlaceOfDeath", FRAME_TEXT},
	{"Profession", FRAME_TEXT_LIST},
	{"PlaceOfResidence", FRAME_RESIDENCES},
	{"Remarks", FRAME_TEXT_LIST},
	{"Images", FRAME_TEXT_LIST},
	{"Audio", FRAME_TEXT},
};
static const size_t thePersonKeyCount = sizeof(thePersonKeys) / sizeof(thePersonKeys[0]);

// In the order of ResidenceKey.
static const KeyInfo theResidenceKeys[] = {
	{"Name", FRAME_TEXT},
	{"StartDate", FRAME_TEXT},
	{"EndDate", FRAME_TEXT},
};
static const size_t theResidenceKeyCount = sizeof(theResidenceKeys) / sizeof(theResidenceKeys[0]);

static void
PrivWarn(Reader* aReader, const char* aBefore, const char* aKey, const char* aAfter) {
	TraceWarning(aReader->trace, "%s: %s\"%s\"%s", aReader->file, aBefore, aKey, aAfter);
	++aReader->warnings;
}

static Frame
PrivFrame(FrameKind aKind, const char* aKey) {
	Frame frame;
	(void)memset(&frame, 0, sizeof(frame));
	frame.kind = aKind;
	frame.key = aKey;
	return frame;
}

// A value of the wrong type arrived for the frame, or an item of the wrong type for its list. It
// is reported, and the frame turns into a skipping one, so that the rest of its value is skipped.
static void
PrivWrongType(Reader* aReader, Frame* aFrame) {
	switch (aFrame->kind) {
	case FRAME_VERSION:
	case FRAME_TEXT:
		PrivWarn(aReader, "key ", aFrame->key, " is not a text, skipped");
		break;
	case FRAME_ID:
		PrivWarn(aReader, "key ", aFrame->key, " is not an integer");
		break;
	case FRAME_TEXT_LIST:
		PrivWarn(aReader, "key ", aFrame->key, " is not a list of texts, skipped");
		break;
	case FRAME_RESIDENCES:
		PrivWarn(aReader, "key ", aFrame->key, " is not a list of Places of Residence, skipped");
		break;
	case FRAME_DOCUMENT:
	case FRAME_PERSON:
	case FRAME_RESIDENCE:
	case FRAME_SKIP:
		break; // the driver finds a wrong document malformed; the others take any value
	}
	aFrame->kind = FRAME_SKIP;
}

// Adds a value to the list of a frame; NULL if memory ran out.
static ListNode*
PrivAddNode(Reader* aReader, Frame* aList) {
	ListNode* node = ArenaAllocate(aReader->scratch, sizeof(ListNode));
	if (node == NULL) {
		aReader->outOfMemory = true;
		return NULL;
	}
	(void)memset(node, 0, sizeof(ListNode));
	node->next = aList->values;
	aList->values = node;
	++aList->count;
	return node;
}

// --- Names for the steps of the Trace

static const char*
PrivTokenName(TokenType aType) {
	switch (aType) {
	case TOKEN_OBJECT_BEGIN:
		return "an opening brace";
	case TOKEN_OBJECT_END:
		return "a closing brace";
	case TOKEN_ARRAY_BEGIN:
		return "an opening bracket";
	case TOKEN_ARRAY_END:
		return "a closing bracket";
	case TOKEN_COLON:
		return "a colon";
	case TOKEN_COMMA:
		return "a comma";
	case TOKEN_STRING:
		return "a string";
	case TOKEN_NUMBER:
		return "a number";
	case TOKEN_LITERAL:
		return "a literal";
	case TOKEN_END:
		return "the end of the text";
	case TOKEN_MALFORMED:
		return "malformed text";
	}
	return "an unknown token";
}

static const char*
PrivFrameName(FrameKind aKind) {
	switch (aKind) {
	case FRAME_DOCUMENT:
		return "document";
	case FRAME_PERSON:
		return "Person";
	case FRAME_RESIDENCE:
		return "Place of Residence";
	case FRAME_VERSION:
		return "Format Version";
	case FRAME_ID:
		return "Person Id";
	case FRAME_TEXT:
		return "text";
	case FRAME_TEXT_LIST:
		return "list of texts";
	case FRAME_RESIDENCES:
		return "list of Places of Residence";
	case FRAME_SKIP:
		return "skipping";
	}
	return "unknown";
}

static const char*
PrivExpectName(Expect aExpect) {
	switch (aExpect) {
	case EXPECT_COLON:
		return "a colon";
	case EXPECT_VALUE:
		return "its value";
	case EXPECT_WAITING:
		return "the end of the object of its value";
	case EXPECT_ENTRY_OR_END:
		return "an entry or its end";
	case EXPECT_ENTRY:
		return "an entry";
	case EXPECT_COMMA_OR_END:
		return "a comma or its end";
	}
	return "something unknown";
}

// --- OnKey: a key arrives in an object, and the frame returns the frame that reads its value.

// The index of the key in the table of keys of an object, or aCount if it is none of them.
static size_t
PrivFindKey(const KeyInfo* aKeys, size_t aCount, const char* aName) {
	for (size_t key = 0; key < aCount; ++key) {
		if (strcmp(aName, aKeys[key].name) == 0) {
			return key;
		}
	}
	return aCount;
}

// The index of the key in the table of keys of the object, if it is one of them and arrives there
// for the first time; otherwise it is reported, and aCount returned.
static size_t
PrivKnownKey(
	Reader* aReader,
	Frame* aObject,
	const KeyInfo* aKeys,
	size_t aCount,
	const char* aName) {
	const size_t key = PrivFindKey(aKeys, aCount, aName);
	if (key == aCount) {
		PrivWarn(aReader, "unknown key ", aName, ", ignored");
		return aCount;
	}
	const unsigned long bit = 1UL << key;
	if ((aObject->seen & bit) != 0) {
		PrivWarn(aReader, "key ", aName, " appears twice, its second value is skipped");
		return aCount;
	}
	aObject->seen |= bit;
	return key;
}

static const char**
PrivPersonTextTarget(FT_PersonRecord* aRecord, PersonKey aKey) {
	switch (aKey) {
	case PERSON_KEY_TITLE:
		return &aRecord->person.title;
	case PERSON_KEY_TITLE_OF_NOBILITY:
		return &aRecord->person.titleOfNobility;
	case PERSON_KEY_GENDER:
		return &aRecord->person.gender;
	case PERSON_KEY_DATE_OF_BIRTH:
		return &aRecord->person.dateOfBirth;
	case PERSON_KEY_PLACE_OF_BIRTH:
		return &aRecord->person.placeOfBirth;
	case PERSON_KEY_DATE_OF_DEATH:
		return &aRecord->person.dateOfDeath;
	case PERSON_KEY_PLACE_OF_DEATH:
		return &aRecord->person.placeOfDeath;
	case PERSON_KEY_AUDIO:
		return &aRecord->recording;
	case PERSON_KEY_VERSION:
	case PERSON_KEY_ID:
	case PERSON_KEY_FIRST_NAMES:
	case PERSON_KEY_LAST_NAMES:
	case PERSON_KEY_PROFESSION:
	case PERSON_KEY_PLACE_OF_RESIDENCE:
	case PERSON_KEY_REMARKS:
	case PERSON_KEY_IMAGES:
		break; // no text
	}
	assert(false && "every key of a text has a target");
	return NULL;
}

static ListTarget
PrivPersonListTarget(FT_PersonRecord* aRecord, PersonKey aKey) {
	FT_TextList* list = NULL;
	switch (aKey) {
	case PERSON_KEY_FIRST_NAMES:
		list = &aRecord->person.firstNames;
		break;
	case PERSON_KEY_LAST_NAMES:
		list = &aRecord->person.lastNames;
		break;
	case PERSON_KEY_PROFESSION:
		list = &aRecord->person.professions;
		break;
	case PERSON_KEY_REMARKS:
		list = &aRecord->person.remarks;
		break;
	case PERSON_KEY_IMAGES: {
		const ListTarget pictures = {&aRecord->pictures, &aRecord->pictureCount};
		return pictures;
	}
	case PERSON_KEY_VERSION:
	case PERSON_KEY_ID:
	case PERSON_KEY_TITLE:
	case PERSON_KEY_TITLE_OF_NOBILITY:
	case PERSON_KEY_GENDER:
	case PERSON_KEY_DATE_OF_BIRTH:
	case PERSON_KEY_PLACE_OF_BIRTH:
	case PERSON_KEY_DATE_OF_DEATH:
	case PERSON_KEY_PLACE_OF_DEATH:
	case PERSON_KEY_PLACE_OF_RESIDENCE:
	case PERSON_KEY_AUDIO:
		break; // no list of texts
	}
	assert(list != NULL && "every key of a list of texts has a target");
	const ListTarget target = {&list->items, &list->count};
	return target;
}

// The frame for the value of a key in the object of the Person; its values go into the record.
static Frame
PrivPersonKey(Reader* aReader, Frame* aPerson, const char* aName) {
	const size_t index = PrivKnownKey(aReader, aPerson, thePersonKeys, thePersonKeyCount, aName);
	if (index == thePersonKeyCount) {
		return PrivFrame(FRAME_SKIP, aName);
	}
	const PersonKey key = (PersonKey)index;
	Frame frame = PrivFrame(thePersonKeys[key].frame, thePersonKeys[key].name);
	FT_PersonRecord* record = aReader->record;
	switch (frame.kind) {
	case FRAME_ID:
		frame.target.id = &record->person.id;
		break;
	case FRAME_TEXT:
		frame.target.text = PrivPersonTextTarget(record, key);
		break;
	case FRAME_TEXT_LIST:
		frame.target.list = PrivPersonListTarget(record, key);
		break;
	case FRAME_RESIDENCES:
		frame.target.residences = &record->person.placesOfResidence;
		break;
	case FRAME_VERSION: // the Format Version goes into the Reader
	case FRAME_DOCUMENT:
	case FRAME_PERSON:
	case FRAME_RESIDENCE:
	case FRAME_SKIP:
		break;
	}
	return frame;
}

static const char**
PrivResidenceTextTarget(FT_Residence* aResidence, ResidenceKey aKey) {
	switch (aKey) {
	case RESIDENCE_KEY_NAME:
		return &aResidence->name;
	case RESIDENCE_KEY_START_DATE:
		return &aResidence->startDate;
	case RESIDENCE_KEY_END_DATE:
		return &aResidence->endDate;
	}
	assert(false && "every key of a Place of Residence has a target");
	return NULL;
}

// The frame for the value of a key in the object of a Place of Residence; its values go into the
// Place of Residence, and every one of them is a text.
static Frame
PrivResidenceKey(Reader* aReader, Frame* aResidence, const char* aName) {
	const size_t index =
		PrivKnownKey(aReader, aResidence, theResidenceKeys, theResidenceKeyCount, aName);
	if (index == theResidenceKeyCount) {
		return PrivFrame(FRAME_SKIP, aName);
	}
	const ResidenceKey key = (ResidenceKey)index;
	Frame frame = PrivFrame(FRAME_TEXT, theResidenceKeys[key].name);
	frame.target.text = PrivResidenceTextTarget(aResidence->target.residence, key);
	return frame;
}

static Frame
PrivOnKey(Reader* aReader, Frame* aFrame, const char* aKey) {
	switch (aFrame->kind) {
	case FRAME_PERSON:
		return PrivPersonKey(aReader, aFrame, aKey);
	case FRAME_RESIDENCE:
		return PrivResidenceKey(aReader, aFrame, aKey);
	case FRAME_SKIP:
		return PrivFrame(FRAME_SKIP, aKey); // the keys of a skipped object are skipped too
	case FRAME_DOCUMENT:
	case FRAME_VERSION:
	case FRAME_ID:
	case FRAME_TEXT:
	case FRAME_TEXT_LIST:
	case FRAME_RESIDENCES:
		break; // never asked: their entries are no keys
	}
	return PrivFrame(FRAME_SKIP, aKey);
}

// --- OnObject: an object arrives as the value of the frame, or as an item of its array, and the
// frame returns the frame that reads it.

// A Place of Residence arrives in the list; it is read into a new value of the list.
static Frame
PrivResidenceFrame(Reader* aReader, Frame* aList) {
	ListNode* node = PrivAddNode(aReader, aList);
	if (node == NULL) {
		return PrivFrame(FRAME_SKIP, aList->key); // the driver stops at outOfMemory
	}
	TraceStep(aReader->trace, "Place of Residence %zu", aList->count);
	Frame frame = PrivFrame(FRAME_RESIDENCE, aList->key);
	frame.target.residence = &node->residence;
	return frame;
}

static Frame
PrivOnObject(Reader* aReader, Frame* aFrame) {
	switch (aFrame->kind) {
	case FRAME_DOCUMENT:
		return PrivFrame(FRAME_PERSON, "");
	case FRAME_RESIDENCES:
		if (aFrame->array) {
			return PrivResidenceFrame(aReader, aFrame);
		}
		break; // holds no list
	case FRAME_TEXT_LIST:
		if (aFrame->array) {
			TraceStep(
				aReader->trace,
				"an object among the texts: the list is skipped when it closes");
			aFrame->wrongItem = true;
			return PrivFrame(FRAME_SKIP, aFrame->key);
		}
		break; // holds no list
	case FRAME_VERSION:
	case FRAME_ID:
	case FRAME_TEXT:
		break;
	case FRAME_SKIP:
	case FRAME_PERSON:
	case FRAME_RESIDENCE:						   // never asked: their entries are keys
		return PrivFrame(FRAME_SKIP, aFrame->key); // what a skipped value holds is skipped too
	}
	PrivWrongType(aReader, aFrame);
	return PrivFrame(FRAME_SKIP, aFrame->key);
}

// --- OnArray: a notice that the value of the frame is an array. aNested: an array arrives as an
// item of its array, which the driver reads with a skipping frame of its own. Returns false if the
// file is malformed.

static bool
PrivOnArray(Reader* aReader, Frame* aFrame, bool aNested) {
	switch (aFrame->kind) {
	case FRAME_TEXT_LIST:
	case FRAME_RESIDENCES:
		if (aNested) {
			TraceStep(
				aReader->trace,
				"an array among the items: the list is skipped when it closes");
			aFrame->wrongItem = true;
			return true;
		}
		TraceStep(aReader->trace, "the value is a list");
		aFrame->array = true; // present, also if it stays empty
		return true;
	case FRAME_VERSION:
	case FRAME_ID:
	case FRAME_TEXT:
		PrivWrongType(aReader, aFrame); // its items are skipped
		return true;
	case FRAME_DOCUMENT:
		TraceStep(aReader->trace, "the document is an array, not an object");
		return false;
	case FRAME_SKIP:
	case FRAME_PERSON:
	case FRAME_RESIDENCE: // never told: their entries are keys
		return true;
	}
	return true;
}

// --- OnValue: a string, number or literal arrives as the value of the frame, or as an item of
// its array. Returns false to stop reading.

// The text of the value of a frame that reads one text; NULL, after a warning, if it is none.
static const char*
PrivText(Reader* aReader, Frame* aFrame, Token aToken) {
	if (aToken.type != TOKEN_STRING) {
		PrivWrongType(aReader, aFrame);
		return NULL;
	}
	if (!aToken.valid) {
		PrivWarn(aReader, "the value of key ", aFrame->key, " is not valid UTF-8, skipped");
		return NULL;
	}
	return aToken.text;
}

// Stops reading at a Format Version the Library does not support, as the rest of the file may
// mean something else (docs/decisions/adr_0020.md).
static bool
PrivReadVersion(Reader* aReader, Frame* aFrame, Token aToken) {
	const char* version = PrivText(aReader, aFrame, aToken);
	if (version == NULL) {
		return true; // reported; the file has no Format Version then
	}
	TraceStep(aReader->trace, "comparing the Format Version with %s", thePersonFileVersion);
	if (!EqualsIgnoringCase(version, thePersonFileVersion)) {
		TraceError(
			aReader->trace,
			"%s: the Format Version %s is not supported",
			aReader->file,
			version);
		aReader->unsupported = true;
		return false;
	}
	aReader->hasVersion = true;
	return true;
}

static void
PrivReadId(Reader* aReader, Frame* aFrame, Token aToken) {
	if (aToken.type != TOKEN_NUMBER) {
		PrivWrongType(aReader, aFrame);
		return;
	}
	FT_PersonId personId = 0;
	if (!ParseInteger(aToken.text, aToken.length, &personId)) {
		PrivWrongType(aReader, aFrame); // e.g. a fraction or an exponent
		return;
	}
	TraceStep(aReader->trace, "writing the Person Id %lld", personId);
	*aFrame->target.id = personId;
	aReader->hasId = true;
}

static void
PrivAddText(Reader* aReader, Frame* aList, Token aToken) {
	if (!aList->array) {
		PrivWrongType(aReader, aList); // a text, not a list, e.g. "FirstNames": "Anna"
		return;
	}
	if (aToken.type != TOKEN_STRING) {
		TraceStep(aReader->trace, "an item is no text: the list is skipped when it closes");
		aList->wrongItem = true;
		return;
	}
	if (!aToken.valid) {
		PrivWarn(aReader, "a value of key ", aList->key, " is not valid UTF-8, skipped");
		return;
	}
	TraceStep(aReader->trace, "adding text %zu", aList->count + 1);
	ListNode* node = PrivAddNode(aReader, aList);
	if (node != NULL) {
		node->text = aToken.text;
	}
}

static bool
PrivOnValue(Reader* aReader, Frame* aFrame, Token aToken) {
	switch (aFrame->kind) {
	case FRAME_TEXT: {
		const char* text = PrivText(aReader, aFrame, aToken);
		if (text == NULL) {
			return true; // reported
		}
		TraceStep(aReader->trace, "writing the text");
		*aFrame->target.text = text;
		return true;
	}
	case FRAME_VERSION:
		return PrivReadVersion(aReader, aFrame, aToken);
	case FRAME_ID:
		PrivReadId(aReader, aFrame, aToken);
		return true;
	case FRAME_TEXT_LIST:
		PrivAddText(aReader, aFrame, aToken);
		return true;
	case FRAME_RESIDENCES:
		if (aFrame->array) {
			TraceStep(aReader->trace, "an item is no object: the list is skipped when it closes");
			aFrame->wrongItem = true;
		} else {
			PrivWrongType(aReader, aFrame);
		}
		return true;
	case FRAME_DOCUMENT:
		TraceStep(aReader->trace, "the document is a single value, not an object");
		return false;
	case FRAME_SKIP:
	case FRAME_PERSON:
	case FRAME_RESIDENCE: // never told: their entries are keys
		return true;
	}
	return true;
}

// --- OnClose: the closing bracket or brace of the frame arrives.

// Turns the values of a list into its array in the arena of the record, allocated once now that
// their number is known; a list with an item of the wrong type stays missing.
static void
PrivCloseTextList(Reader* aReader, Frame* aList) {
	if (aList->wrongItem) {
		PrivWrongType(aReader, aList);
		return;
	}
	TraceStep(aReader->trace, "copying %zu texts into the record", aList->count);
	const char** items =
		(const char**)ArenaAllocate(aReader->arena, (aList->count + 1) * sizeof(const char*));
	if (items == NULL) {
		aReader->outOfMemory = true;
		return;
	}
	size_t index = aList->count;
	// Ends at the end of the list of values, which are newest first.
	for (const ListNode* node = aList->values; node != NULL; node = node->next) {
		--index;
		items[index] = node->text;
	}
	*aList->target.list.items = items;
	*aList->target.list.count = aList->count;
}

static void
PrivCloseResidences(Reader* aReader, Frame* aList) {
	if (aList->wrongItem) {
		PrivWrongType(aReader, aList);
		return;
	}
	TraceStep(aReader->trace, "copying %zu Places of Residence into the record", aList->count);
	FT_Residence* items = ArenaAllocate(aReader->arena, (aList->count + 1) * sizeof(FT_Residence));
	if (items == NULL) {
		aReader->outOfMemory = true;
		return;
	}
	size_t index = aList->count;
	// Ends at the end of the list of values, which are newest first.
	for (const ListNode* node = aList->values; node != NULL; node = node->next) {
		--index;
		items[index] = node->residence;
	}
	aList->target.residences->items = items;
	aList->target.residences->count = aList->count;
}

static void
PrivOnClose(Reader* aReader, Frame* aFrame) {
	switch (aFrame->kind) {
	case FRAME_TEXT_LIST:
		PrivCloseTextList(aReader, aFrame);
		return;
	case FRAME_RESIDENCES:
		PrivCloseResidences(aReader, aFrame);
		return;
	case FRAME_DOCUMENT:
	case FRAME_PERSON:
	case FRAME_RESIDENCE:
	case FRAME_VERSION:
	case FRAME_ID:
	case FRAME_TEXT:
	case FRAME_SKIP:
		return; // their values are written as they arrive
	}
}

// =================================================================================================
// 2. Driver
// =================================================================================================

static Frame*
PrivTop(Reader* aReader) {
	assert(aReader->depth > 0 && "the stack is not empty");
	return &aReader->frames[aReader->depth - 1];
}

static bool
PrivPush(Reader* aReader, Frame aFrame, Expect aExpect, bool aIsArray) {
	if (aReader->depth == JSON_MAX_DEPTH) {
		return false; // nested too deeply: malformed
	}
	aFrame.expect = aExpect;
	aFrame.isArray = aIsArray;
	aFrame.warningsBefore = aReader->warnings;
	aReader->frames[aReader->depth] = aFrame;
	++aReader->depth;
	// Every frame reads its part in a Subtrace of its own, so that a part read without problems is
	// compacted as soon as it is complete, and the Trace keeps the shape of the stack.
	const char* name = PrivFrameName(aFrame.kind);
	if (aFrame.key[0] == '\0') {
		TraceOpen(aReader->trace, name, "");
		return true;
	}
	TraceOpen(aReader->trace, name, "\"%s\"", aFrame.key);
	return true;
}

// The result of the frame on top, which read its part: failed if memory ran out, with warnings if
// any were written while it was on the stack.
static Result
PrivFrameResult(Reader* aReader) {
	if (aReader->outOfMemory) {
		return RESULT_OUT_OF_MEMORY;
	}
	if (aReader->warnings > PrivTop(aReader)->warningsBefore) {
		return RESULT_OK_WITH_WARNINGS;
	}
	return RESULT_OK;
}

// Pops the frame on top, and closes its Subtrace with aResult.
static void
PrivPopWith(Reader* aReader, Result aResult) {
	(void)TraceCloseResult(aReader->trace, aResult);
	--aReader->depth;
}

// Pops the frame on top, which read its part, and closes its Subtrace with its result.
static void
PrivPop(Reader* aReader) {
	PrivPopWith(aReader, PrivFrameResult(aReader));
}

// An entry of the frame on top is complete. A frame that waited for its one value is complete with
// it, and is popped, which completes an entry of the frame below; a frame with several entries goes
// on after one.
static void
PrivEntryDone(Reader* aReader) {
	// Only a bound: each pass pops a frame, or ends.
	for (size_t pass = 0; pass < JSON_MAX_DEPTH && aReader->depth > 0; ++pass) {
		Frame* top = PrivTop(aReader);
		if (top->expect != EXPECT_VALUE && top->expect != EXPECT_WAITING) {
			top->expect = EXPECT_COMMA_OR_END;
			return;
		}
		PrivPop(aReader);
	}
}

// A value for the frame on top: as the value it waits for, or as an item of its array.
static bool
PrivValue(Reader* aReader, Token aToken) {
	Frame* top = PrivTop(aReader);
	switch (aToken.type) {
	case TOKEN_OBJECT_BEGIN: {
		const Frame object = PrivOnObject(aReader, top);
		if (top->expect == EXPECT_VALUE) {
			top->expect = EXPECT_WAITING;
		}
		return PrivPush(aReader, object, EXPECT_ENTRY_OR_END, false);
	}
	case TOKEN_ARRAY_BEGIN:
		// An array gets no frame of its own: the frame that waits for a value becomes the array.
		// One already in an array has to hand an array in an array to a frame of its own.
		if (top->isArray) {
			TraceStep(aReader->trace, "an array in an array: read by a skipping frame of its own");
			if (!PrivOnArray(aReader, top, true)) {
				return false;
			}
			return PrivPush(aReader, PrivFrame(FRAME_SKIP, top->key), EXPECT_ENTRY_OR_END, true);
		}
		top->isArray = true;
		top->expect = EXPECT_ENTRY_OR_END;
		return PrivOnArray(aReader, top, false);
	case TOKEN_STRING:
	case TOKEN_NUMBER:
	case TOKEN_LITERAL:
		if (!PrivOnValue(aReader, top, aToken)) {
			return false;
		}
		PrivEntryDone(aReader);
		return true;
	case TOKEN_OBJECT_END:
	case TOKEN_ARRAY_END:
	case TOKEN_COLON:
	case TOKEN_COMMA:
	case TOKEN_END:
	case TOKEN_MALFORMED:
		break;
	}
	return false; // malformed
}

// Whether a token is the bracket or brace that closes the frame on top.
static bool
PrivIsClosing(const Frame* aTop, Token aToken) {
	return aToken.type == (aTop->isArray ? TOKEN_ARRAY_END : TOKEN_OBJECT_END);
}

// Closes and pops the frame on top, which completes an entry of the frame below.
static bool
PrivClose(Reader* aReader) {
	PrivOnClose(aReader, PrivTop(aReader));
	PrivPop(aReader);
	PrivEntryDone(aReader);
	return true;
}

// The colon after a key.
static bool
PrivColon(Reader* aReader, Token aToken) {
	if (aToken.type != TOKEN_COLON) {
		return false;
	}
	PrivTop(aReader)->expect = EXPECT_VALUE;
	return true;
}

// An entry of the frame on top: a value in an array, or a key in an object, whose frame then waits
// for the colon. A key that is not valid UTF-8 is skipped with its value, like any other value
// that is not valid UTF-8 (docs/requirements/req_0051.md).
static bool
PrivEntry(Reader* aReader, Token aToken) {
	Frame* top = PrivTop(aReader);
	if (top->isArray) {
		return PrivValue(aReader, aToken);
	}
	if (aToken.type != TOKEN_STRING) {
		return false;
	}
	if (!aToken.valid) {
		TraceWarning(aReader->trace, "%s: a key is not valid UTF-8, ignored", aReader->file);
		++aReader->warnings;
		return PrivPush(aReader, PrivFrame(FRAME_SKIP, ""), EXPECT_COLON, false);
	}
	return PrivPush(aReader, PrivOnKey(aReader, top, aToken.text), EXPECT_COLON, false);
}

// The first entry of an object or an array, or its closing bracket or brace if it is empty.
static bool
PrivEntryOrEnd(Reader* aReader, Token aToken) {
	if (PrivIsClosing(PrivTop(aReader), aToken)) {
		return PrivClose(aReader);
	}
	return PrivEntry(aReader, aToken);
}

// The comma before the next entry, or the closing bracket or brace after the last one.
static bool
PrivCommaOrEnd(Reader* aReader, Token aToken) {
	Frame* top = PrivTop(aReader);
	if (aToken.type == TOKEN_COMMA) {
		top->expect = EXPECT_ENTRY;
		return true;
	}
	if (PrivIsClosing(top, aToken)) {
		return PrivClose(aReader);
	}
	return false;
}

// One token, handed to the step of the grammar the frame on top waits for. Returns false if the
// file is malformed, or reading stops.
static bool
PrivStep(Reader* aReader, Token aToken) {
	switch (PrivTop(aReader)->expect) {
	case EXPECT_COLON:
		return PrivColon(aReader, aToken);
	case EXPECT_VALUE:
		return PrivValue(aReader, aToken);
	case EXPECT_ENTRY_OR_END:
		return PrivEntryOrEnd(aReader, aToken);
	case EXPECT_ENTRY:
		return PrivEntry(aReader, aToken);
	case EXPECT_COMMA_OR_END:
		return PrivCommaOrEnd(aReader, aToken);
	case EXPECT_WAITING:
		break; // never on top: the frame of its object is
	}
	return false;
}

// The step for a token that arrives: where it is, what it is, and where the frame on top is in the
// grammar; the Subtrace it is written into names the frame. A string that is not valid UTF-8 is not
// written into the Trace.
static void
PrivTraceToken(Reader* aReader, Token aToken, size_t aPosition) {
	const char* expect = PrivExpectName(PrivTop(aReader)->expect);
	if (aToken.type == TOKEN_STRING && aToken.valid) {
		TraceStep(aReader->trace, "byte %zu: \"%s\", expecting %s", aPosition, aToken.text, expect);
		return;
	}
	if (aToken.type == TOKEN_NUMBER || aToken.type == TOKEN_LITERAL) {
		TraceStep(
			aReader->trace,
			"byte %zu: %.*s, expecting %s",
			aPosition,
			(int)aToken.length,
			aToken.text,
			expect);
		return;
	}
	TraceStep(
		aReader->trace,
		"byte %zu: %s, expecting %s",
		aPosition,
		PrivTokenName(aToken.type),
		expect);
}

// Reads the document from the frame of the document on. Returns whether it is complete and
// nothing follows it; aOutPosition is where the last token read starts, e.g. a malformed one.
static bool
PrivReadDocument(Reader* aReader, char* aText, size_t* aOutPosition) {
	(void)PrivPush(aReader, PrivFrame(FRAME_DOCUMENT, ""), EXPECT_VALUE, false);
	const size_t length = strlen(aText);
	char* position = aText;
	// Only a bound: each pass reads one token, and every token but the end consumes at least one
	// character.
	for (size_t pass = 0; pass <= length; ++pass) {
		const Token token = PrivNextToken(position);
		*aOutPosition = (size_t)(token.start - aText);
		if (aReader->depth == 0) {
			TraceStep(
				aReader->trace,
				"byte %zu: %s after the complete document",
				*aOutPosition,
				PrivTokenName(token.type));
			return token.type == TOKEN_END; // only the end of the text may follow
		}
		// The end of the text before the document is complete is refused by every step.
		PrivTraceToken(aReader, token, *aOutPosition);
		const bool goesOn = PrivStep(aReader, token);
		if (!goesOn || aReader->outOfMemory) {
			return false; // malformed, or reading stopped
		}
		position = token.next;
	}
	return false;
}

// Reports why reading stopped before the document was complete, and returns the failure. It is
// written while the frames are still open, so that their Subtraces show where it stopped.
static Result
PrivReportStop(Reader* aReader, size_t aPosition) {
	if (aReader->outOfMemory) {
		TraceError(aReader->trace, "%s: memory ran out while parsing", aReader->file);
		return RESULT_OUT_OF_MEMORY;
	}
	if (aReader->unsupported) {
		return RESULT_UNREADABLE; // reported at the Format Version
	}
	TraceError(aReader->trace, "%s: not valid JSON near byte %zu", aReader->file, aPosition);
	return RESULT_UNREADABLE;
}

// Pops the frames still open where reading stopped, and closes their Subtraces with the failure,
// as none of them could read its part; so the path to where it stopped keeps every step.
static void
PrivPopAll(Reader* aReader, Result aFailure) {
	// Only a bound: each pass pops a frame.
	for (size_t pass = 0; pass < JSON_MAX_DEPTH && aReader->depth > 0; ++pass) {
		PrivPopWith(aReader, aFailure);
	}
}

Result
ParsePersonFile(
	char* aText,
	const char* aFile,
	Arena* aScratch,
	Arena* aArena,
	FT_PersonRecord* aOutRecord,
	FT_Trace* aTrace) {
	TraceOpen(aTrace, "ParsePersonFile", "%s", aFile);
	(void)memset(aOutRecord, 0, sizeof(*aOutRecord));
	if (HasOtherByteOrderMark(aText)) {
		TraceError(aTrace, "%s: the file is UTF-16 or UTF-32, save it as UTF-8", aFile);
		return TraceCloseResult(aTrace, RESULT_UNREADABLE);
	}
	const size_t byteOrderMark = ByteOrderMarkLength(aText);
	TraceStep(aTrace, "reading the document after a byte order mark of %zu bytes", byteOrderMark);
	Reader reader;
	(void)memset(&reader, 0, sizeof(reader));
	reader.record = aOutRecord;
	reader.file = aFile;
	reader.scratch = aScratch;
	reader.arena = aArena;
	reader.trace = aTrace;
	size_t position = 0;
	const bool read = PrivReadDocument(&reader, aText + byteOrderMark, &position);
	if (!read) {
		const Result failure = PrivReportStop(&reader, position);
		PrivPopAll(&reader, failure);
		return TraceCloseResult(aTrace, failure);
	}
	if (!reader.hasVersion) {
		TraceError(aTrace, "%s: it has no Format Version", aFile);
		return TraceCloseResult(aTrace, RESULT_UNREADABLE);
	}
	if (!reader.hasId) {
		TraceError(aTrace, "%s: it has no integer Id", aFile);
		return TraceCloseResult(aTrace, RESULT_UNREADABLE);
	}
	return TraceCloseResult(aTrace, reader.warnings > 0 ? RESULT_OK_WITH_WARNINGS : RESULT_OK);
}
