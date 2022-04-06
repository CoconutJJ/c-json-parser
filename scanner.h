#ifndef scanner_h
#define scanner_h
#include <stdlib.h>

#define TOKEN_TYPE(token) token.type;

typedef enum {
        LBRACE,
        RBRACE,
        LBRACKET,
        RBRACKET,
        COLON,
        COMMA,
        LONG,
        DOUBLE,
        STRING,
        TRUE,
        FALSE,
        NIL,
        END,
        ERROR
} TokenType;

typedef struct {
        TokenType type;
        size_t line_no;
        size_t col_no;
        union {
                struct {
                        union {
                                long l;
                                double d;
                        };
                        long exp;
                };

                char *s;
                char *error_message;
        };
} Token;

Token scan_token ();
void init_json_scanner (char *input);
#endif