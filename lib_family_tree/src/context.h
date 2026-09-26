#ifndef FT_CONTEXT_H
#define FT_CONTEXT_H

#include <family_tree/api.h>
#include <stddef.h>

struct Context {
	size_t relationCount;
	RelationExt* relations;
	size_t personCount;
	PersonExt* persons;

	size_t allocatedStringCount;
	Utf8_String* allocatedStrings;
};

#endif // FT_CONTEXT_H