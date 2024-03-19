#pragma once
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    union {
        void* ptr;
        const char* chars;
    };
    size_t num_chars;
} string_t;

#define MAKE_STRING(CHARS) ((string_t) { .chars = (CHARS), .num_chars = strlen((CHARS)) })

bool string_equal(string_t a, string_t b);
bool string_lower_equal(string_t a, string_t b);
