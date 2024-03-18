#pragma once
#include "types.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    string_t str;
    // Must be null-terminated st strlen(delim) >= 1
    const char* delim;
} lexer_info_t;

typedef struct {
    size_t index;
    size_t num_chars;
} lexer_t;

lexer_t init_lexer(const lexer_info_t* info);
bool check_lexer(const lexer_info_t* info, const lexer_t* lexer);
lexer_t next_lexer(const lexer_info_t* info, const lexer_t* lexer);
