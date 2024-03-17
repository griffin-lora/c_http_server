#include "str.h"
#include <string.h>

string_t get_next_token(string_t token, const char* delim) {
    token.chars += token.num_chars;
    token.chars += strspn(token.chars, delim);
 
    if (*token.chars == '\0') {
        return (string_t) { .ptr = NULL } ;
    }
 
    token.num_chars = strcspn(token.chars, delim);
 
    return token;
}
