#pragma once
#include "result.h"
#include "types.h"
#include <stddef.h>

typedef enum {
    http_content_type_text_plain
} http_content_type_t;

typedef struct {
    int dummy;
} http_request_header_t;

typedef enum {
    http_request_type_get,
    http_request_type_post
} http_request_type_t;

typedef struct {
    http_request_type_t type;
    string_t path;
    http_request_header_t header;
} http_request_t;

result_t parse_http_request_message(string_t request_msg, http_request_t* request);

typedef struct {
    http_content_type_t content_type;
} http_response_header_t;

typedef enum {
    http_response_type_ok
} http_response_type_t;

typedef struct {
    http_response_type_t type;
    string_t content;
    http_response_header_t header;
} http_response_t;

void create_http_response_message(const http_response_t* response, string_t* response_msg);
