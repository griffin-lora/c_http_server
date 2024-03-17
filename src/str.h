#pragma once
#include <stddef.h>

typedef struct {
    union {
        void* ptr;
        const char* chars;
    };
    size_t num_chars;
} string_t;

string_t get_next_token(string_t token, const char* delim);

static inline string_t get_first_token(const char* str, const char* delim) {
    return get_next_token((string_t) {
        .chars = str,
        .num_chars = 0
    }, delim);
}
