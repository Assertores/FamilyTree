// Text helpers: UTF-8, byte order marks, paths and integers.

#include "internal.h"

#include <family_tree/api.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// The bytes of UTF-8 sequences (RFC 3629).
enum {
	UTF8_CONTINUATION_FIRST = 0x80,
	UTF8_CONTINUATION_LAST = 0xBF,
	UTF8_TWO_FIRST = 0xC2,
	UTF8_TWO_LAST = 0xDF,
	UTF8_THREE_FIRST = 0xE0,	  // its second byte starts at 0xA0, to exclude overlong sequences
	UTF8_THREE_SURROGATES = 0xED, // its second byte ends at 0x9F, to exclude surrogates
	UTF8_THREE_LAST = 0xEF,
	UTF8_FOUR_FIRST = 0xF0, // its second byte starts at 0x90, to exclude overlong sequences
	UTF8_FOUR_LAST = 0xF4,	// its second byte ends at 0x8F, to stay below U+110000
	UTF8_AFTER_THREE_FIRST = 0xA0,
	UTF8_AFTER_SURROGATES = 0x9F,
	UTF8_AFTER_FOUR_FIRST = 0x90,
	UTF8_AFTER_FOUR_LAST = 0x8F,
	DECIMAL_BASE = 10,
};

// The length of a sequence that starts with aLead, and the range of its second byte; false if
// aLead cannot start a sequence.
typedef struct {
	size_t length;
	unsigned char secondFirst;
	unsigned char secondLast;
} SequenceShape;

static bool
PrivShapeOf(unsigned char aLead, SequenceShape* aOut) {
	aOut->secondFirst = UTF8_CONTINUATION_FIRST;
	aOut->secondLast = UTF8_CONTINUATION_LAST;
	if (aLead >= UTF8_TWO_FIRST && aLead <= UTF8_TWO_LAST) {
		aOut->length = 2;
	} else if (aLead >= UTF8_THREE_FIRST && aLead <= UTF8_THREE_LAST) {
		aOut->length = 3;
		if (aLead == UTF8_THREE_FIRST) {
			aOut->secondFirst = UTF8_AFTER_THREE_FIRST;
		} else if (aLead == UTF8_THREE_SURROGATES) {
			aOut->secondLast = UTF8_AFTER_SURROGATES;
		}
	} else if (aLead >= UTF8_FOUR_FIRST && aLead <= UTF8_FOUR_LAST) {
		aOut->length = 4;
		if (aLead == UTF8_FOUR_FIRST) {
			aOut->secondFirst = UTF8_AFTER_FOUR_FIRST;
		} else if (aLead == UTF8_FOUR_LAST) {
			aOut->secondLast = UTF8_AFTER_FOUR_LAST;
		}
	} else {
		return false;
	}
	return true;
}

static bool
PrivIsContinuation(unsigned char aByte) {
	return aByte >= UTF8_CONTINUATION_FIRST && aByte <= UTF8_CONTINUATION_LAST;
}

size_t
Utf8SequenceLength(const char* aText) {
	const unsigned char lead = (unsigned char)aText[0];
	if (lead < UTF8_CONTINUATION_FIRST) {
		return 1;
	}
	SequenceShape shape = {0, 0, 0};
	if (!PrivShapeOf(lead, &shape)) {
		return 0;
	}
	// A zero byte is no continuation, so the checks stop at the end of the text.
	const unsigned char second = (unsigned char)aText[1];
	if (second < shape.secondFirst || second > shape.secondLast) {
		return 0;
	}
	for (size_t i = 2; i < shape.length; ++i) {
		if (!PrivIsContinuation((unsigned char)aText[i])) {
			return 0;
		}
	}
	return shape.length;
}

bool
IsValidUtf8(const char* aText) {
	const size_t length = strlen(aText);
	for (size_t position = 0, sequence = 0; position < length; position += sequence) {
		sequence = Utf8SequenceLength(aText + position);
		if (sequence == 0) {
			return false;
		}
	}
	return true;
}

bool
StartsWith(const char* aText, const char* aStart) {
	return strncmp(aText, aStart, strlen(aStart)) == 0;
}

size_t
ByteOrderMarkLength(const char* aText) {
	return StartsWith(aText, "\xEF\xBB\xBF") ? 3 : 0;
}

bool
HasOtherByteOrderMark(const char* aText) {
	// UTF-32 starts like UTF-16 in little endian, and with zeros, i.e. an empty text, in big
	// endian.
	return StartsWith(aText, "\xFF\xFE") || StartsWith(aText, "\xFE\xFF");
}

static char
PrivLowerCase(char aCharacter) {
	if (aCharacter >= 'A' && aCharacter <= 'Z') {
		return (char)(aCharacter - 'A' + 'a');
	}
	return aCharacter;
}

bool
EqualsIgnoringCase(const char* aFirst, const char* aSecond) {
	const size_t length = strlen(aFirst);
	if (strlen(aSecond) != length) {
		return false;
	}
	for (size_t i = 0; i < length; ++i) {
		if (PrivLowerCase(aFirst[i]) != PrivLowerCase(aSecond[i])) {
			return false;
		}
	}
	return true;
}

static bool
PrivNeedsSeparator(const char* aFolder, size_t aLength) {
	return aLength > 0 && aFolder[aLength - 1] != '/' && aFolder[aLength - 1] != '\\';
}

char*
JoinPath(Arena* aArena, const char* aFolder, const char* aFirst, const char* aSecond) {
	const size_t folderLength = strlen(aFolder);
	const size_t firstLength = strlen(aFirst);
	const size_t secondLength = aSecond != NULL ? strlen(aSecond) : 0;
	const bool separator = PrivNeedsSeparator(aFolder, folderLength);
	char* path = ArenaAllocate(aArena, folderLength + firstLength + secondLength + 3);
	if (path == NULL) {
		return NULL;
	}
	size_t used = folderLength;
	(void)memcpy(path, aFolder, folderLength);
	if (separator) {
		path[used] = '/';
		++used;
	}
	(void)memcpy(path + used, aFirst, firstLength);
	used += firstLength;
	if (aSecond != NULL) {
		path[used] = '/';
		++used;
		(void)memcpy(path + used, aSecond, secondLength);
		used += secondLength;
	}
	path[used] = '\0';
	return path;
}

bool
ParseInteger(const char* aText, size_t aLength, FT_PersonId* aOut) {
	const bool negative = aLength > 0 && aText[0] == '-';
	const size_t first = negative ? 1 : 0;
	if (aLength <= first) {
		return false;
	}
	// The magnitude of LLONG_MIN is one more than LLONG_MAX.
	const unsigned long long limit =
		negative ? (unsigned long long)LLONG_MAX + 1 : (unsigned long long)LLONG_MAX;
	unsigned long long magnitude = 0;
	for (size_t i = first; i < aLength; ++i) {
		if (aText[i] < '0' || aText[i] > '9') {
			return false;
		}
		const unsigned long long digit = (unsigned long long)(aText[i] - '0');
		if (magnitude > (limit - digit) / DECIMAL_BASE) {
			return false;
		}
		magnitude = (magnitude * DECIMAL_BASE) + digit;
	}
	if (negative) {
		*aOut = magnitude == limit ? LLONG_MIN : -(FT_PersonId)magnitude;
	} else {
		*aOut = (FT_PersonId)magnitude;
	}
	return true;
}
