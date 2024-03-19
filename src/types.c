#include "types.h"

bool string_equal(string_t a, string_t b) {
    if (a.num_chars != b.num_chars) {
        return false;
    }
    for (size_t i = 0; i < a.num_chars; i++) {
        if (a.chars[i] != b.chars[i]) {
            return false;
        }
    }
    return true;
}
