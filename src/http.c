#include "http.h"
#include "str.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char* delim = "\r\n";

result_t parse_http_request_message(const char* request_msg, http_request_t* request) {
    (void)request;
    string_t token = get_first_token(request_msg, delim);
    while (token.ptr != NULL) {
        printf("Token: %.*s\n", (int) token.num_chars, token.chars);
        token = get_next_token(token, delim);
    }

    return result_success;
}
