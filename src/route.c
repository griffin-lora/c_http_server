#include "route.h"
#include "http.h"
#include "types.h"
#include <stdio.h>

#define KEEP_ALIVE_TIMEOUT_SECONDS 30

extern const char _binary_resource_index_html_start[];
extern const char _binary_resource_index_html_end[];

http_response_t get_response_for_request(const http_request_t* request) {
    const string_t index_content = {
        .num_chars = (size_t) (_binary_resource_index_html_end - _binary_resource_index_html_start),
        .chars = _binary_resource_index_html_start
    };
    const string_t error_content = MAKE_STRING("Error");

    printf("%.*s\n", (int) request->resource_path.num_chars, request->resource_path.chars);

    http_response_t response = {
        .type = http_response_type_ok,
        .content = string_equal(request->resource_path, MAKE_STRING("/index.html")) ? index_content : error_content,
        .header = {
            .content_type = http_content_type_text_html,
            .connection_type = request->header.connection_type,
            .keep_alive_timeout_seconds = KEEP_ALIVE_TIMEOUT_SECONDS
        }
    };

    return response;
}