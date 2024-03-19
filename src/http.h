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
    const string_t* headers_key;
    const string_t* headers_value;
} http_request_t;

result_t parse_http_request_message(string_t request_msg, http_request_t* request);

typedef enum {
    http_response_type_ok = 200
} http_response_type_t;

typedef struct {
    http_response_type_t type;
    string_t content;
    size_t num_headers;
    const string_t* headers_key;
    const string_t* headers_value;
} http_response_t;

void create_http_response_message(const http_response_t* response, string_t* response_msg);
