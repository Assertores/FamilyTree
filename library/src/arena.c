// Arena: every allocation is one node of a list, freed only as a whole or back to a mark
// (docs/decisions/adr_0019.md).

#include "internal.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// The types with the strictest alignment. As the header of a node is as large as this union, the
// bytes after it suit every type, like the memory malloc returns.
typedef union {
	long double longDouble;
	long long longLong;
	void* pointer;
	double floating;
} ArenaAlignment;

// The header of an allocation; the bytes asked for follow it.
struct ArenaNode {
	union {
		ArenaNode* previous;
		ArenaAlignment alignment;
	} header;
};

void*
ArenaAllocate(Arena* aArena, size_t aSize) {
	assert(aArena != NULL);
	if (aSize > SIZE_MAX - sizeof(ArenaNode)) {
		return NULL;
	}
	ArenaNode* node = malloc(sizeof(ArenaNode) + aSize);
	if (node == NULL) {
		return NULL;
	}
	node->header.previous = aArena->last;
	aArena->last = node;
	return node + 1;
}

char*
ArenaCopyText(Arena* aArena, const char* aText) {
	assert(aText != NULL);
	const size_t length = strlen(aText);
	char* copy = ArenaAllocate(aArena, length + 1);
	if (copy != NULL) {
		(void)memcpy(copy, aText, length + 1);
	}
	return copy;
}

ArenaMark
ArenaGetMark(const Arena* aArena) {
	const ArenaMark mark = {aArena->last};
	return mark;
}

// The loops over the nodes have no counted bound (docs/decisions/adr_0011.md, Rule 2), as an
// Arena does not count its nodes. They are bounded by the number of nodes all the same: every node
// links only to a node allocated before it, so the list has no cycle and ends with NULL after at
// most as many nodes as were allocated.

void
ArenaReset(Arena* aArena, ArenaMark aMark) {
	// Each pass frees one node; the mark is a node of this list, or NULL, so it is reached.
	while (aArena->last != aMark.last) {
		ArenaNode* node = aArena->last;
		assert(node != NULL); // the mark belongs to another Arena
		aArena->last = node->header.previous;
		free(node);
	}
}

void
ArenaMoveAll(Arena* aFrom, Arena* aTo) {
	// Linking an Arena to itself would make a cycle, which no loop over it could leave.
	assert(aFrom != aTo);
	if (aFrom->last == NULL) {
		return;
	}
	ArenaNode* oldest = aFrom->last;
	for (; oldest->header.previous != NULL; oldest = oldest->header.previous) {
	}
	oldest->header.previous = aTo->last;
	aTo->last = aFrom->last;
	aFrom->last = NULL;
}

void
ArenaFree(Arena* aArena) {
	const ArenaMark empty = {NULL};
	ArenaReset(aArena, empty);
}
