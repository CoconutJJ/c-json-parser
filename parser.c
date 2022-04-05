#include "parser.h"
#include "mem.h"
#include "scanner.h"
#include <stdbool.h>
#include <stdlib.h>

Token curr = { 0 };

bool has_errors = false;

void advance ()
{
        Token next = scan_token ();

        if (has_errors)
                return;

        if (next.type == ERROR) {
                has_errors = true;
        }

        curr = next;
}

Token peek ()
{
        return curr;
}

bool match (TokenType t)
{
        if (has_errors)
                return false;

        if (peek ().type == t) {
                advance ();
                return true;
        }

        return false;
}

JSON_ITEM *create_json_item (JSON_ITEMTYPE type)
{
        JSON_ITEM *item = allocate (sizeof (JSON_ITEM));
        item->obj.type = ITEM;
        item->item_type = type;

        return item;
}

JSON_OBJECT *create_json_object ()
{
        JSON_OBJECT *obj = allocate (sizeof (JSON_OBJECT));
        obj->obj.type = OBJECT;
        obj->count = 0;
        obj->capacity = 1;
        obj->entries = allocate (sizeof (JSON_ENTRY *));

        return obj;
}

JSON_ARRAY *create_json_array ()
{
        JSON_ARRAY *arr = allocate (sizeof (JSON_ARRAY));
        arr->obj.type = ARRAY;
        arr->count = 0;
        arr->capacity = 1;
        arr->items = allocate (sizeof (JSON_ITEM *));

        return arr;
}

JSON_ENTRY *create_json_entry (char *key, JSON *value)
{
        JSON_ENTRY *ent = allocate (sizeof (JSON_ENTRY));

        ent->obj.type = ENTRY;
        ent->key = key;
        ent->value = value;

        return ent;
}

void add_json_entry (JSON_OBJECT *obj, JSON_ENTRY *new_entry)
{
        if (obj->count == obj->capacity) {
                obj->capacity *= 2;
                reallocate (obj->entries, obj->capacity * sizeof (JSON_ENTRY *));
        }

        obj->entries[obj->count++] = new_entry;
}

void add_json_array (JSON_ARRAY *arr, JSON *new_item)
{
        if (arr->count == arr->capacity) {
                arr->capacity *= 2;
                reallocate (arr->items, arr->capacity * sizeof (JSON *));
        }

        arr->items[arr->count++] = new_item;
}

void consume (TokenType t, char *message, ...)
{
        if (!match (t))
                has_errors = true;
}

void destroy_json (JSON *json)
{
        if (!json)
                return;

        switch (json->type) {
        case ARRAY: {
                JSON_ARRAY *arr = AS_JSON_ARRAY (json);

                for (size_t i = 0; i < arr->count; i++)
                        destroy_json (AS_JSON (arr->items[i]));

                free (arr->items);
                free (arr);
                break;
        }
        case ENTRY: {
                JSON_ENTRY *ent = AS_JSON_ENTRY (json);

                if (ent->key)
                        free (ent->key);

                destroy_json (ent->value);
                free (ent);

                break;
        }
        case OBJECT: {
                JSON_OBJECT *obj = AS_JSON_OBJECT (json);

                for (size_t i = 0; i < obj->count; i++)
                        destroy_json (AS_JSON (obj->entries[i]));

                free (obj->entries);
                free (obj);
                break;
        }

        case ITEM: {
                JSON_ITEM *item = AS_JSON_ITEM (json);

                if (item->item_type == JSON_TYPESTRING && item->s)
                        free (item->s);

                free (item);
                break;
        }
        default: break;
        }
}

JSON *parse ()
{
        Token t = peek ();
        advance ();
        switch (t.type) {
        case LBRACE: {
                JSON_OBJECT *obj = create_json_object ();

                if (match (RBRACE)) {
                        return AS_JSON (obj);
                }

                while (1) {
                        Token key = peek ();
                        consume (STRING, "expected key after opening `{`");
                        consume (COLON, "expected `:` after key");

                        JSON *value = parse ();

                        if (!value)
                                return NULL;

                        add_json_entry (obj, create_json_entry (key.s, value));

                        if (match (COMMA))
                                continue;

                        if (match (RBRACE))
                                break;

                        return NULL;
                }

                return AS_JSON (obj);
        }
        case LBRACKET: {
                JSON_ARRAY *arr = create_json_array ();

                if (match (RBRACKET)) {
                        return AS_JSON (arr);
                }

                while (1) {
                        JSON *item = parse ();

                        if (!item)
                                return NULL;

                        add_json_array (arr, item);

                        if (match (RBRACKET))
                                break;
                        if (match (COMMA))
                                continue;

                        return NULL;
                }

                return AS_JSON (arr);
        }
        case STRING: {
                JSON_ITEM *item = create_json_item (JSON_TYPESTRING);
                item->s = t.s;
                return AS_JSON (item);
        }
        case LONG: {
                JSON_ITEM *item = create_json_item (JSON_TYPELONG);
                item->l = t.l;
                return AS_JSON (item);
        }
        case DOUBLE: {
                JSON_ITEM *item = create_json_item (JSON_TYPEDOUBLE);
                item->d = t.d;
                return AS_JSON (item);
        }
        default: break;
        }
        return NULL;
}

JSON *parse_json (char *input)
{
        init_json_scanner (input);
        advance ();

        JSON *json = parse ();

        if (has_errors) {
                destroy_json (json);
                return NULL;
        }
        return json;
}