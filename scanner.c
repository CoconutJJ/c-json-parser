#include "scanner.h"
#include "mem.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *input_buffer = NULL;
static char *curr = NULL;

static size_t col_no = 0;
static size_t line_no = 1;

static bool at_end ()
{
        return *curr == '\0';
}

static void advance ()
{
        curr++;
}

static char peek ()
{
        return *curr;
}

static bool match (char c)
{
        if (peek () == c) {
                advance ();
                return true;
        }

        return false;
}

static bool is_numeric (char c)
{
        return '0' <= c && c <= '9';
}

static bool is_alpha (char c)
{
        return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

static bool exponent (Token *n)
{
        char *start = curr;

        if (!match ('-') && !match ('+'))
                return false;

        while (is_numeric (peek ()))
                advance ();

        n->exp = strtol (start, NULL, 10);

        return true;
}

static Token number ()
{
        char *start = curr - 1;

        while (is_numeric (peek ()))
                advance ();

        Token n;
        n.col_no = col_no;
        n.line_no = line_no;
        n.exp = 0;

        if (!match ('.')) {
                n.type = LONG;
                n.l = strtol (start, NULL, 10);

                if (match ('e') || match ('E'))
                        exponent (&n);

                return n;
        }

        while (is_numeric (peek ()))
                advance ();

        char *end = curr;
        char buf[end - start + 1];
        strncpy (buf, start, end - start);
        buf[end - start] = '\0';

        n.type = DOUBLE;
        n.d = strtod (buf, NULL);

        if (match ('e') || match ('E'))
                exponent (&n);

        return n;
}

static Token create_error_token (char *error_message, size_t arg_size, ...)
{
        char *error_buf = allocate (strlen (error_message) + arg_size + 1);
        va_list args;
        va_start (args, arg_size);
        vsprintf (error_buf, error_message, args);
        va_end (args);
        Token error = { .type = ERROR, .col_no = col_no, .line_no = line_no, .error_message = error_buf };

        return error;
}

static Token literal ()
{
        char *start = curr - 1;

        while (is_alpha (peek ()))
                advance ();

        char *end = curr;

        char *literal_buf = allocate (end - start + 1);

        strncpy (literal_buf, start, end - start);
        literal_buf[end - start] = '\0';

        if (strcmp ("true", literal_buf) == 0)
                return (Token){ .type = TRUE, .col_no = col_no, .line_no = line_no };
        else if (strcmp ("false", literal_buf) == 0)
                return (Token){ .type = FALSE, .col_no = col_no, .line_no = line_no };
        else if (strcmp ("null", literal_buf) == 0)
                return (Token){ .type = NIL, .col_no = col_no, .line_no = line_no };
        else
                return create_error_token ("unexpected literal", 0);
}

static Token string ()
{
        char *start = curr;
        while (!match ('"')) {
                if (at_end ())
                        return create_error_token ("unexpected EOF, was expecting CLOSING `\"`", 0);

                if (match ('\\'))
                        advance ();

                advance ();
        }

        char *end_quote = curr - 1;

        char *buf = allocate ((end_quote - start + 1) * sizeof (char));

        strncpy (buf, start, end_quote - start);
        buf[end_quote - start] = '\0';

        Token s;
        s.type = STRING;
        s.line_no = line_no;
        s.col_no = col_no;
        s.s = buf;

        return s;
}

void init_json_scanner (char *input)
{
        input_buffer = input;
        curr = input;
        col_no = 0;
        line_no = 1;
}

Token scan_token ()
{
#define ERROR_TOKEN(message)                                                                                           \
        (Token)                                                                                                        \
        {                                                                                                              \
                .type = ERROR, .line_no = line_no, .col_no = col_no, .s = message                                      \
        }

        for (;;) {
                if (at_end ())
                        return (Token){ .type = END, .col_no = 1 };

                char c = peek ();
                advance ();

                Token token;

                col_no++;

                switch (c) {
                case '{': token.type = LBRACE; break;
                case '}': token.type = RBRACE; break;
                case '[': token.type = LBRACKET; break;
                case ']': token.type = RBRACKET; break;
                case ',': token.type = COMMA; break;
                case ':': token.type = COLON; break;
                case '\"': token = string (); break;
                case '\n':
                        line_no++;
                        col_no = 1;
                        break;
                case ' ':
                case '\r':
                case '\t': continue;
                default:
                        if (is_numeric (c) || c == '-' || c == '+')
                                token = number ();
                        else if (is_alpha (c))
                                token = literal ();
                        else
                                token = create_error_token ("unknown symbol %c", c);

                        break;
                }
                return token;
        }
}
