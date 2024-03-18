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
        printf("Line token: %.*s\n", (int) lexer.num_chars, token);
        switch (i) {
            case 0:
                if (strncmp(token, "GET", lexer.num_chars) == 0) {
                    *type = http_request_type_get;
                    break;
                }
                if (strncmp(token, "POST", lexer.num_chars) == 0) {
                    *type = http_request_type_post;
                    break;
                }
                break;
            case 1:
                *path = (string_t) {
                    .num_chars = lexer.num_chars,
                    .chars = token 
                };
                break;
            case 2:
                return strncmp(token, "HTTP/1.1", lexer.num_chars) == 0 ? result_success : result_failure;
            default:
                return result_failure;
        } 
    }

    return result_failure;
}

static result_t parse_header(size_t num_line_chars, const char line[], string_t* key, string_t* value) {
    lexer_info_t lexer_info = {
        .num_chars = num_line_chars,
        .str = line,
        .delim = ": "
    };
    
    lexer_t lexer = init_lexer(&lexer_info);
    {
        const char* token = line + lexer.index;
        printf("Key: %.*s\n", (int) lexer.num_chars, token);
        *key = (string_t) {
            .num_chars = lexer.num_chars,
            .chars = token
        };
    }

    if (!check_lexer(&lexer_info, &lexer)) {
        return result_failure;
    }
    lexer = next_lexer(&lexer_info, &lexer); 

    {
        const char* token = line + lexer.index;
        printf("Value: %.*s\n", (int) (num_line_chars - lexer.index), token);
        *value = (string_t) {
            .num_chars = num_line_chars - lexer.index,
            .chars = token
        };
    } 

    return result_success;

}

result_t parse_http_request_message(const char* request_msg, http_request_t* request) {
    http_request_type_t type;
    string_t path;

    lexer_info_t lexer_info = {
        .num_chars = strlen(request_msg),
        .str = request_msg,
        .delim = "\r\n"
    };

    lexer_t header_lexer;

    size_t i = 0;
    for (
        lexer_t lexer = init_lexer(&lexer_info);
        check_lexer(&lexer_info, &lexer);
        lexer = next_lexer(&lexer_info, &lexer), i++
    ) {
        const char* token = request_msg + lexer.index;
        printf("Token: %.*s\n", (int) lexer.num_chars, token); 
        if (i > 0) {
            continue;
        }
        if (parse_request_first_line(lexer.num_chars, token, &type, &path) != result_success) {
            return result_failure;
        }
        header_lexer = next_lexer(&lexer_info, &lexer);
    }
    
    if (i <= 1) {
        return result_failure;
    }

    size_t num_headers = i - 1;
    string_t* headers_key = malloc(sizeof(string_t) * num_headers);
    string_t* headers_value = malloc(sizeof(string_t) * num_headers);

    i = 0;
    for (
        lexer_t lexer = header_lexer;
        check_lexer(&lexer_info, &lexer);
        lexer = next_lexer(&lexer_info, &lexer), i++
    ) {
        const char* token = request_msg + lexer.index;  
        if (parse_header(lexer.num_chars, token, &headers_key[i], &headers_value[i]) != result_success) {
            return result_failure;
        }
    }

    *request = (http_request_t) {
        .type = type,
        .path = path,
        .num_headers = num_headers,
        .headers_key = headers_key,
        .headers_value = headers_value
    };

    return result_success;
}
