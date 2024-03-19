#pragma once
#include <stddef.h>

typedef struct {
    union {
        void* ptr;
        const char* chars;
    };
    size_t num_chars;
} string_t;

#define MAKE_STRING(CHARS) ((string_t) { .chars = (CHARS), .num_chars = strlen((CHARS)) })
