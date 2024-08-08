#ifndef FT_JSON_PARSER_H
#define FT_JSON_PARSER_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct JsonParsingDispatchTable {
	struct JsonParsingDispatchTable (*getKeyHandler)(const char* aKey);
	void (*parseInt)(int aValue);
	void (*parseString)(const char* aValue);
} JsonParsingDispatchTable;

void ParseJson(char* aJson, JsonParsingDispatchTable aHandler);

#ifdef __cplusplus
}
#endif
#endif // FT_JSON_PARSER_H