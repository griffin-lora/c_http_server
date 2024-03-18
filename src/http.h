#pragma once
#include "result.h"
#include "types.h"
#include <stddef.h>

typedef enum {
    http_request_type_get,
    http_request_type_post
} http_request_type_t;

typedef struct {
    http_request_type_t type;
    string_t path;
    size_t num_headers;
    string_t* headers_key;
    string_t* headers_value;
} http_request_t;

result_t parse_http_request_message(const char* request_msg, http_request_t* request);