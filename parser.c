#include "parser.h"
#include "mem.h"
#include "scanner.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
static Token curr = { 0 };

static bool has_errors = false;

static void advance ()
{
        Token next = scan_token ();

        if (has_errors)
                return;

        if (next.type == ERROR) {
                has_errors = true;
        }

        curr = next;
}

static Token peek ()
{
        return curr;
}

static bool match (TokenType t)
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
                obj->entries = reallocate (obj->entries, obj->capacity * sizeof (JSON_ENTRY *));
        }

        obj->entries[obj->count++] = new_entry;
}

void add_json_array (JSON_ARRAY *arr, JSON *new_item)
{
        if (arr->count == arr->capacity) {
                arr->capacity *= 2;
                arr->items = reallocate (arr->items, arr->capacity * sizeof (JSON *));
        }

        arr->items[arr->count++] = new_item;
}

static void consume (TokenType t, char *message, ...)
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

static JSON *parse ()
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
                                break;

                        add_json_entry (obj, create_json_entry (key.s, value));

                        if (match (COMMA))
                                continue;

                        if (match (RBRACE))
                                break;

                        has_errors = true;
                        break;
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
                                break;

                        add_json_array (arr, item);

                        if (match (RBRACKET))
                                break;
                        if (match (COMMA))
                                continue;

                        has_errors = true;
                        break;
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
                item->exp = t.exp;
                return AS_JSON (item);
        }
        case DOUBLE: {
                JSON_ITEM *item = create_json_item (JSON_TYPEDOUBLE);
                item->d = t.d;
                item->exp = t.exp;
                return AS_JSON (item);
        }
        case TRUE: {
                JSON_ITEM *item = create_json_item (JSON_TYPETRUE);
                return AS_JSON (item);
        }
        case FALSE: {
                JSON_ITEM *item = create_json_item (JSON_TYPEFALSE);
                return AS_JSON (item);
        }
        case NIL: {
                JSON_ITEM *item = create_json_item (JSON_TYPENIL);
                return AS_JSON (item);
        }
        default: break;
        }
        return NULL;
}

void print_json (JSON *json)
{
        switch (json->type) {
        case ITEM: {
                JSON_ITEM *item = AS_JSON_ITEM (json);

                switch (item->item_type) {
                case JSON_TYPEDOUBLE: {
                        printf ("%.2f", item->d);
                        if (item->exp != 0) {
                                putchar ('E');
                                putchar (item->exp > 0 ? '+' : '-');
                                printf ("%ld", item->exp);
                        }
                        break;
                }
                case JSON_TYPELONG: {
                        printf ("%ld", item->l);
                        if (item->exp != 0) {
                                putchar ('E');
                                putchar (item->exp > 0 ? '+' : '-');
                                printf ("%ld", item->exp);
                        }
                        break;
                }
                case JSON_TYPESTRING: printf ("\"%s\"", item->s); break;
                case JSON_TYPEFALSE: printf ("false"); break;
                case JSON_TYPETRUE: printf ("true"); break;
                case JSON_TYPENIL: printf ("null"); break;
                default: break;
                }
                break;
        }
        case ARRAY: {
                JSON_ARRAY *arr = AS_JSON_ARRAY (json);
                printf ("[");
                for (int i = 0; i < arr->count; i++) {
                        print_json (arr->items[i]);

                        if (i != arr->count - 1)
                                printf (", ");
                }
                printf ("]");

                break;
        }

        case OBJECT: {
                JSON_OBJECT *obj = AS_JSON_OBJECT (json);
                printf ("{");
                for (int i = 0; i < obj->count; i++) {
                        print_json (AS_JSON (obj->entries[i]));

                        if (i != obj->count - 1)
                                printf (", ");
                }
                printf ("}");
                break;
        }
        case ENTRY: {
                JSON_ENTRY *ent = AS_JSON_ENTRY (json);
                printf ("\"%s\":", ent->key);
                print_json (ent->value);
                break;
        }
        default: break;
        }
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