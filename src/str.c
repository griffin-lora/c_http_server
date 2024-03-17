#include "str.h"
#include <string.h>

string_t tokenize_string(string_t str, const char* delim) {
    str.chars += str.num_chars;
    str.chars += strspn(str.chars, delim);
 
    if (*str.chars == '\0') {
        return (string_t) { .ptr = NULL } ;
    }
 
    str.num_chars = strcspn(str.chars, delim);
 
    return str;
}
