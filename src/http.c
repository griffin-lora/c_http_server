#include "http.h"
#include "result.h"
#include "types.h"
#include "token.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static result_t parse_request_resource_path(string_t request_resource_path, string_t* resource_path) {
    lexer_info_t lexer_info = {
        .str = request_resource_path,
        .delim = "?"
    };

    lexer_t lexer = init_lexer(&lexer_info);
    *resource_path = get_token(&lexer_info, &lexer);

    return result_success;
}

static result_t parse_request_first_line(string_t line, http_request_type_t* type, string_t* resource_path) {
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
        string_t token = get_token(&lexer_info, &lexer);
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
            case 1: {
                result_t result = parse_request_resource_path(token, resource_path);
                if (result != result_success) {
                    return result;
                }
            } break;
            case 2:
                return string_equal(token, MAKE_STRING("HTTP/1.1")) ? result_success : result_invalid_first_line;
            default:
                return result_invalid_first_line;
        } 
    }

    return result_invalid_first_line;
}

static result_t parse_connection_type(string_t str, http_connection_type_t* type) {
    if (string_equal(str, MAKE_STRING("keep-alive"))) {
        *type = http_connection_type_keep_alive;
        return result_success;
    } else if (string_equal(str, MAKE_STRING("close"))) {
        *type = http_connection_type_close;
        return result_success;
    }
    return result_invalid_connection_type;
}

static result_t parse_header_line(string_t line, http_request_header_t* header) {
    lexer_info_t lexer_info = {
        .str = line,
        .delim = ": "
    };
    
    lexer_t lexer = init_lexer(&lexer_info);
    string_t key = get_token(&lexer_info, &lexer); 

    if (!check_lexer(&lexer_info, &lexer)) {
        return result_invalid_header_line;
    }
    lexer = next_lexer(&lexer_info, &lexer); 
    string_t value = get_token(&lexer_info, &lexer);

    if (string_lower_equal(key, MAKE_STRING("Connection"))) {
        result_t result = parse_connection_type(value, &header->connection_type);
        if (parse_connection_type(value, &header->connection_type) != result_success) { return result; }
    }
    return result_success;
}

result_t parse_http_request_message(string_t request_msg, http_request_t* request) {
    http_request_type_t type;
    string_t resource_path;

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
        if (i > 0) {
            continue;
        }
        result_t result = parse_request_first_line(get_token(&lexer_info, &lexer), &type, &resource_path);
        if (result != result_success) { return result; }
        header_lexer = next_lexer(&lexer_info, &lexer);
    }
    
    if (i <= 1) {
        return result_invalid_http_request_message;
    }

    http_request_header_t header;
    memset(&header, 0, sizeof(header));

    i = 0;
    for (
        lexer_t lexer = header_lexer;
        check_lexer(&lexer_info, &lexer);
        lexer = next_lexer(&lexer_info, &lexer), i++
    ) {
        result_t result = parse_header_line(get_token(&lexer_info, &lexer), &header);
        if (result != result_success) { return result; }
    }

    *request = (http_request_t) {
        .type = type,
        .resource_path = resource_path,
        .header = header
    };

    return result_success;
}

static const char* get_response_type_string(http_response_type_t type) {
    switch (type) {
        case http_response_type_ok: return "200 OK";
        default: return NULL;
    }
}

static const char* get_content_type_string(http_content_type_t type) {
    switch (type) {
        case http_content_type_text_plain: return "text/plain";
        case http_content_type_text_html: return "text/html";
        default: return NULL;
    }
}

static const char* get_connection_type_string(http_connection_type_t type) {
    switch (type) {
        case http_connection_type_keep_alive: return "keep-alive";
        case http_connection_type_close: return "close";
        default: return NULL;
    }
}

void create_http_response_message(const http_response_t* response, string_t* response_msg) {
    #define FORMAT_BASE_STRING_PREFIX \
        "HTTP/1.1 %s\r\n" \
        "Content-Length: %d\r\n" \
        "Content-Type: %s\r\n" \
        "Connection: %s\r\n"

    #define FORMAT_BASE_ARGS_PREFIX \
        get_response_type_string(response->type), \
        (int) response->content.num_chars, \
        get_content_type_string(response->header.content_type), \
        get_connection_type_string(response->header.connection_type),

    #define FORMAT_BASE_STRING_SUFFIX \
        "\r\n%.*s\r\n", 

    #define FORMAT_BASE_ARGS_SUFFIX \
        (int) response->content.num_chars, response->content.chars


    #define CONNECTION_TYPE_CLOSE_FORMAT \
        FORMAT_BASE_STRING_PREFIX \
        FORMAT_BASE_STRING_SUFFIX \
        FORMAT_BASE_ARGS_PREFIX \
        FORMAT_BASE_ARGS_SUFFIX


    #define CONNECTION_TYPE_KEEP_ALIVE_FORMAT \
        FORMAT_BASE_STRING_PREFIX \
        "Keep-Alive: timeout=%lu\r\n" \
        FORMAT_BASE_STRING_SUFFIX \
        FORMAT_BASE_ARGS_PREFIX \
        response->header.keep_alive_timeout_seconds, \
        FORMAT_BASE_ARGS_SUFFIX
    
    size_t num_response_msg_chars = 0;
    switch (response->header.connection_type) {
        case http_connection_type_close:
            num_response_msg_chars = (size_t) snprintf(NULL, 0, CONNECTION_TYPE_CLOSE_FORMAT);
            break;
        case http_connection_type_keep_alive:
            num_response_msg_chars = (size_t) snprintf(NULL, 0, CONNECTION_TYPE_KEEP_ALIVE_FORMAT);
            break;
    }

    char* response_msg_chars = malloc(num_response_msg_chars + 1);
    
    switch (response->header.connection_type) {
        case http_connection_type_close:
            sprintf(response_msg_chars, CONNECTION_TYPE_CLOSE_FORMAT);
            break;
        case http_connection_type_keep_alive:
            sprintf(response_msg_chars, CONNECTION_TYPE_KEEP_ALIVE_FORMAT);
            break;
    }

    *response_msg = (string_t) {
        .num_chars = num_response_msg_chars,
        .chars = response_msg_chars
    };
}
