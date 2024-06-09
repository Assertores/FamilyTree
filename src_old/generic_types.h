#ifndef GENERIC_TYPES_H
#define GENERIC_TYPES_H
#include "base_types.h"

#include <stdlib.h>
#include <string.h>

#define Array(Type)                                               \
	typedef struct {                                              \
		Type* data;                                               \
		size_t length;                                            \
	} Type##s;                                                    \
	Bool Append##Type(Type##s* aArray, Type aDataToAppend);       \
	Bool Append##Type##s(Type##s* aArray, Type##s aDataToAppend); \
	void Free##Type##s(Type##s aArray);

#define ArrayImpl(Type, FreeFunc)                                            \
	Bool Append##Type(Type##s* aArray, Type aDataToAppend) {                 \
		aArray->length++;                                                    \
		aArray->data = realloc(aArray->data, aArray->length * sizeof(Type)); \
		aArray->data[aArray->length - 1] = aDataToAppend;                    \
		return True;                                                         \
	}                                                                        \
	Bool Append##Type##s(Type##s* aArray, Type##s aDataToAppend) {           \
		aArray->length += aDataToAppend.length;                              \
		aArray->data = realloc(aArray->data, aArray->length * sizeof(Type)); \
		memcpy(                                                              \
			aArray->data + aArray->length - aDataToAppend.length,            \
			aDataToAppend.data,                                              \
			aDataToAppend.length * sizeof(Type));                            \
		free(aDataToAppend.data);                                            \
		return True;                                                         \
	}                                                                        \
	void Free##Type##s(Type##s aArray) {                                     \
		for (size_t i = 0; i < aArray.length; i++) {                         \
			FreeFunc(aArray.data[i]);                                        \
		}                                                                    \
		if (aArray.length > 0) {                                             \
			free(aArray.data);                                               \
		}                                                                    \
		aArray.length = 0;                                                   \
	}

#define ArrayRef(Type)                                            \
	typedef struct {                                              \
		Type** data;                                              \
		size_t length;                                            \
	} Type##s;                                                    \
	Bool Append##Type(Type##s* aArray, Type* aDataToAppend);      \
	Bool Append##Type##s(Type##s* aArray, Type##s aDataToAppend); \
	void Free##Type##s(Type##s aArray);

#define ArrayRefImpl(Type)                                                    \
	Bool Append##Type(Type##s* aArray, Type* aDataToAppend) {                 \
		aArray->length++;                                                     \
		aArray->data = realloc(aArray->data, aArray->length * sizeof(Type*)); \
		aArray->data[aArray->length - 1] = aDataToAppend;                     \
		return True;                                                          \
	}                                                                         \
	Bool Append##Type##s(Type##s* aArray, Type##s aDataToAppend) {            \
		aArray->length += aDataToAppend.length;                               \
		aArray->data = realloc(aArray->data, aArray->length * sizeof(Type*)); \
		memcpy(                                                               \
			aArray->data + aArray->length - aDataToAppend.length,             \
			aDataToAppend.data,                                               \
			aDataToAppend.length * sizeof(Type));                             \
		Free##Type##s(aDataToAppend);                                         \
		return True;                                                          \
	}                                                                         \
	void Free##Type##s(Type##s aArray) {                                      \
		if (aArray.length > 0) {                                              \
			free(aArray.data);                                                \
		}                                                                     \
		aArray.length = 0;                                                    \
	}

#define Set(Type)                                               \
	typedef struct {                                            \
		Type* data;                                             \
		size_t length;                                          \
	} Type##s;                                                  \
	Bool Append##Type(Type##s* aSet, Type aDataToAppend);       \
	Bool Append##Type##s(Type##s* aSet, Type##s aDataToAppend); \
	void Free##Type##s(Type##s aSet);

#define SetImpl(Type, CompareFunc, FreeFunc)                           \
	Bool Append##Type(Type##s* aSet, Type aDataToAppend) {             \
		for (size_t i = 0; i < aSet->length; i++) {                    \
			if (CompareFunc(&aSet->data[i], &aDataToAppend) == True) { \
				return Duplicate;                                      \
			}                                                          \
		}                                                              \
		aSet->length++;                                                \
		aSet->data = realloc(aSet->data, aSet->length * sizeof(Type)); \
		aSet->data[aSet->length - 1] = aDataToAppend;                  \
		return True;                                                   \
	}                                                                  \
	Bool Append##Type##s(Type##s* aSet, Type##s aDataToAppend) {       \
		for (size_t i = 0; i < aDataToAppend.length; i++) {            \
			Bool result = Append##Type(aSet, aDataToAppend.data[i]);   \
			if (result != True) {                                      \
				return result;                                         \
			}                                                          \
		}                                                              \
		free(aDataToAppend.data);                                      \
		return True;                                                   \
	}                                                                  \
	void Free##Type##s(Type##s aSet) {                                 \
		for (size_t i = 0; i < aSet.length; i++) {                     \
			FreeFunc(aSet.data[i]);                                    \
		}                                                              \
		if (aSet.length > 0) {                                         \
			free(aSet.data);                                           \
		}                                                              \
		aSet.length = 0;                                               \
	}

#define SetRef(Type)                                            \
	typedef struct {                                            \
		Type** data;                                            \
		size_t length;                                          \
	} Type##s;                                                  \
	Bool Append##Type(Type##s* aSet, Type* aDataToAppend);      \
	Bool Append##Type##s(Type##s* aSet, Type##s aDataToAppend); \
	void Free##Type##s(Type##s aSet);

#define SetRefImpl(Type, Compare)                                      \
	Bool Append##Type(Type##s* aSet, Type* aDataToAppend) {            \
		for (size_t i = 0; i < aSet->length; i++) {                    \
			if (Compare(aSet->data[i], aDataToAppend) == True) {       \
				return Duplicate;                                      \
			}                                                          \
		}                                                              \
		aSet->length++;                                                \
		aSet->data = realloc(aSet->data, aSet->length * sizeof(Type)); \
		aSet->data[aSet->length - 1] = aDataToAppend;                  \
		return True;                                                   \
	}                                                                  \
	Bool Append##Type##s(Type##s* aSet, Type##s aDataToAppend) {       \
		for (size_t i = 0; i < aDataToAppend.length; i++) {            \
			Bool result = Append##Type(aSet, aDataToAppend.data[i]);   \
			if (result != True) {                                      \
				return result;                                         \
			}                                                          \
		}                                                              \
		Free##Type##s(aDataToAppend);                                  \
		return True;                                                   \
	}                                                                  \
	void Free##Type##s(Type##s aSet) {                                 \
		if (aSet.length > 0) {                                         \
			free(aSet.data);                                           \
		}                                                              \
		aSet.length = 0;                                               \
	}

#define Expected(Type) \
	typedef struct {   \
		Bool hasData;  \
		Type data;     \
	} E##Type;

#define ExpectedRef(Type) \
	typedef struct {      \
		Bool hasData;     \
		Type* data;       \
	} E##Type;
#endif // GENERIC_TYPES_H
