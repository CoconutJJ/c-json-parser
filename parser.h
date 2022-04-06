#ifndef parser_h
#define parser_h
#include <stdlib.h>

#define AS_JSON(json)        ((JSON *)(json))
#define AS_JSON_ITEM(json)   ((JSON_ITEM *)(json))
#define AS_JSON_OBJECT(json) ((JSON_OBJECT *)(json))
#define AS_JSON_ARRAY(json)  ((JSON_ARRAY *)(json))
#define AS_JSON_ENTRY(json)  ((JSON_ENTRY *)(json))

typedef enum { ITEM, OBJECT, ARRAY, ENTRY } JSON_OBJTYPE;

typedef enum {
        JSON_TYPESTRING,
        JSON_TYPELONG,
        JSON_TYPEDOUBLE,
        JSON_TYPETRUE,
        JSON_TYPEFALSE,
        JSON_TYPENIL
} JSON_ITEMTYPE;

typedef struct {
        JSON_OBJTYPE type;
} JSON;

typedef struct {
        JSON obj;
        JSON_ITEMTYPE item_type;
        union {
                char *s;
                struct {
                        union {
                                long l;
                                double d;
                        };
                        long exp;
                };
        };
} JSON_ITEM;

typedef struct {
        JSON obj;
        char *key;
        JSON *value;
} JSON_ENTRY;

typedef struct {
        JSON obj;
        size_t capacity;
        size_t count;
        JSON_ENTRY **entries;
} JSON_OBJECT;

typedef struct {
        JSON obj;
        size_t capacity;
        size_t count;
        JSON **items;
} JSON_ARRAY;

JSON *parse_json (char *input);
void print_json (JSON *json);
#endif