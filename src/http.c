#include "http.h"
#include "types.h"
#include "token.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static result_t parse_request_first_line(size_t num_line_chars, const char line[], http_request_type_t* type, string_t* path) {
    lexer_info_t lexer_info = {
        .num_chars = num_line_chars,
        .str = line,
        .delim = " "
    };

    size_t i = 0;
    for (
        lexer_t lexer = init_lexer(&lexer_info);
        check_lexer(&lexer_info, &lexer);
        lexer = next_lexer(&lexer_info, &lexer), i++
    ) {
        const char* token = line + lexer.index;
        printf("Token: %.*s\n", (int) lexer.num_chars, token);
        switch (i) {
            case 0:
                // TODO: Implement correctly
                *type = http_request_type_get;
                break;
            case 1:
                // TODO: Be careful here, the path is now only guaranteed to be allocated for function scope
                *path = (string_t) {
                    .num_chars = lexer.num_chars,
                    .chars = token 
                };
                break;
            case 2:
                if (strncmp(token, "HTTP/1.1", lexer.num_chars) == 0) {
                    return result_success;
                } else {
                    return result_failure;
                }
                break;
        } 
    }

    return result_failure;
}

result_t parse_http_request_message(const char* request_msg, http_request_t* request) {
    http_request_type_t type;
    string_t path;

    lexer_info_t lexer_info = {
        .num_chars = strlen(request_msg),
        .str = request_msg,
        .delim = "\r\n"
    };

    size_t i = 0;
    for (
        lexer_t lexer = init_lexer(&lexer_info);
        check_lexer(&lexer_info, &lexer);
        lexer = next_lexer(&lexer_info, &lexer), i++
    ) {
        const char* token = request_msg + lexer.index;
        printf("Lexer: %lu, %lu\n", lexer.num_chars, lexer.index);
        printf("Token: %.*s\n", (int) lexer.num_chars, token); 
        if (i == (size_t)-1) {
            if (parse_request_first_line(lexer.num_chars, token, &type, &path) != result_success) {
                return result_failure;
            }
        }
    }

    *request = (http_request_t) {
        .type = type,
        .path = path
    };

    return result_success;
}
