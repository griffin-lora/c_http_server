#pragma once
#include <stddef.h>

typedef struct {
    union {
        void* ptr;
        const char* chars;
    };
    size_t num_chars;
} string_t;
