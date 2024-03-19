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

static result_t parse_header_line(string_t line, http_request_header_t* header) {
    lexer_info_t lexer_info = {
        .str = line,
        .delim = ": "
    };
    
    lexer_t lexer = init_lexer(&lexer_info);
    string_t key = {
        .chars = line.chars + lexer.index,
        .num_chars = lexer.num_chars
    }; 

    if (!check_lexer(&lexer_info, &lexer)) {
        return result_failure;
    }
    lexer = next_lexer(&lexer_info, &lexer); 

    string_t value = {
        .chars = line.chars + lexer.index,
        .num_chars = lexer.num_chars
    };

    // TODO: Implement actual header parsing
    (void)key;
    (void)value;
    (void)header;
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

    http_request_header_t header;
    memset(&header, 0, sizeof(header));

    i = 0;
    for (
        lexer_t lexer = header_lexer;
        check_lexer(&lexer_info, &lexer);
        lexer = next_lexer(&lexer_info, &lexer), i++
    ) {
        const char* token_chars = request_msg.chars + lexer.index;  
        if (parse_header_line((string_t) {
            .num_chars = lexer.num_chars,
            .chars = token_chars
        }, &header) != result_success) {
            return result_failure;
        }
    }

    *request = (http_request_t) {
        .type = type,
        .path = path,
        .header = header
    };

    return result_success;
}

static const char* get_response_type_string(http_response_type_t type) {
    switch (type) {
        case http_response_type_ok: return "200 OK";
    }
    return NULL;
}

static const char* get_content_type_string(http_content_type_t type) {
    switch (type) {
        case http_content_type_text_plain: return "text/plain";
    }
    return NULL;
}

void create_http_response_message(const http_response_t* response, string_t* response_msg) {
    #define FORMAT_ARGS \
        "HTTP/1.1 %s\r\n" \
        "Content-Length: %d\r\n" \
        "Content-Type: %s\r\n" \
        "\r\n%.*s\r\n", \
        get_response_type_string(response->type), \
        (int) response->content.num_chars, \
        get_content_type_string(response->header.content_type), \
        (int) response->content.num_chars, response->content.chars

    size_t num_response_msg_chars = 0;
    num_response_msg_chars += (size_t) snprintf(NULL, 0, FORMAT_ARGS);

    char* response_msg_chars = malloc(num_response_msg_chars + 1);
    
    sprintf(response_msg_chars, FORMAT_ARGS); 

    #undef FORMAT_ARGS

    *response_msg = (string_t) {
        .num_chars = num_response_msg_chars,
        .chars = response_msg_chars
    };
}
