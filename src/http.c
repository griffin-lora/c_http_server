#include "http.h"
#include "types.h"
#include "token.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static result_t parse_request_first_line(string_t line, http_request_type_t* type, string_t* path) {
    lexer_info_t lexer_info = {
        .str = line,
        .delim = " "
    };

    size_t i = 0;
    for (
        lexer_t lexer = init_lexer(&lexer_info);
        check_lexer(&lexer_info, &lexer);
        lexer = next_lexer(&lexer_info, &lexer), i++
    ) {
        string_t token = {
            .chars = line.chars + lexer.index,
            .num_chars = lexer.num_chars
        };
        switch (i) {
            case 0:
                if (string_equal(token, MAKE_STRING("GET"))) {
                    *type = http_request_type_get;
                    break;
                }
                if (string_equal(token, MAKE_STRING("POST"))) {
                    *type = http_request_type_post;
                    break;
                }
                break;
            case 1:
                *path = token;
                break;
            case 2:
                return string_equal(token, MAKE_STRING("HTTP/1.1")) ? result_success : result_failure;
            default:
                return result_failure;
        } 
    }

    return result_failure;
}

static result_t parse_header(string_t line, string_t* key, string_t* value) {
    lexer_info_t lexer_info = {
        .str = line,
        .delim = ": "
    };
    
    lexer_t lexer = init_lexer(&lexer_info);
    {
        string_t token = {
            .chars = line.chars + lexer.index,
            .num_chars = lexer.num_chars
        };
        *key = token;
    }

    if (!check_lexer(&lexer_info, &lexer)) {
        return result_failure;
    }
    lexer = next_lexer(&lexer_info, &lexer); 

    {
        string_t token = {
            .chars = line.chars + lexer.index,
            .num_chars = lexer.num_chars
        };
        *value = token;
    } 

    return result_success;

}

result_t parse_http_request_message(string_t request_msg, http_request_t* request) {
    http_request_type_t type;
    string_t path;

    lexer_info_t lexer_info = {
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
        const char* token_chars = request_msg.chars + lexer.index;
        printf("Token: %.*s\n", (int) lexer.num_chars, token_chars); 
        if (i > 0) {
            continue;
        }
        if (parse_request_first_line((string_t) {
            .num_chars = lexer.num_chars,
            .chars = token_chars
        }, &type, &path) != result_success) {
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
        const char* token_chars = request_msg.chars + lexer.index;  
        if (parse_header((string_t) {
            .num_chars = lexer.num_chars,
            .chars = token_chars
        }, &headers_key[i], &headers_value[i]) != result_success) {
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

void create_http_response_message(const http_response_t* response, string_t* response_msg) {
    size_t num_response_msg_chars = 0;
    num_response_msg_chars += (size_t) snprintf(NULL, 0, "HTTP/1.1 %d OK\r\n\r\n%.*s\r\n", (int) response->type, (int) response->content.num_chars, response->content.chars);

    for (size_t i = 0; i < response->num_headers; i++) {
        const string_t* key = &response->headers_key[i];
        const string_t* value = &response->headers_value[i];
        num_response_msg_chars += (size_t) snprintf(NULL, 0, "%.*s: %.*s\r\n",
            (int) key->num_chars,
            key->chars,
            (int) value->num_chars,
            value->chars
        );
    }

    char* response_msg_chars = malloc(num_response_msg_chars + 1);
    
    size_t index = 0;
    index += (size_t) sprintf(&response_msg_chars[index], "HTTP/1.1 %d OK\r\n", (int) response->type);
    
    for (size_t i = 0; i < response->num_headers; i++) {
        const string_t* key = &response->headers_key[i];
        const string_t* value = &response->headers_value[i];
        index += (size_t) sprintf(&response_msg_chars[index], "%.*s: %.*s\r\n",
            (int) key->num_chars,
            key->chars,
            (int) value->num_chars,
            value->chars
        );
    }

    index += (size_t) sprintf(&response_msg_chars[index], "\r\n%.*s\r\n", (int) response->content.num_chars, response->content.chars);

    *response_msg = (string_t) {
        .num_chars = num_response_msg_chars,
        .chars = response_msg_chars
    };
}
