#include "route.h"
#include "http.h"
#include "types.h"
#include <pthread.h>
#include <stdio.h>

#define KEEP_ALIVE_TIMEOUT_SECONDS 30

#define MAKE_RESOURCE(NAME) \
extern const char _binary_resource_##NAME##_start[]; \
extern const char _binary_resource_##NAME##_end[]; \

#define MAKE_RESOURCE_STRING(NAME) \
const string_t NAME##_content = { \
    .num_chars = (size_t) (_binary_resource_##NAME##_end - _binary_resource_##NAME##_start), \
    .chars = _binary_resource_##NAME##_start \
};

MAKE_RESOURCE(not_found_html)
MAKE_RESOURCE(index_html)

http_response_t get_response_for_request(const http_request_t* request) {
    MAKE_RESOURCE_STRING(index_html)
    MAKE_RESOURCE_STRING(not_found_html)

    printf("Resource path requested (Thread 0x%lx): %.*s\n", pthread_self(), (int) request->resource_path.num_chars, request->resource_path.chars);

    http_response_type_t response_type;
    string_t content;
    http_content_type_t content_type;

    if (string_equal(request->resource_path, MAKE_STRING("/index.html"))) {
        response_type = http_response_type_ok;
        content = index_html_content;
        content_type = http_content_type_text_html;
    } else {
        response_type = http_response_type_not_found;
        content = not_found_html_content;
        content_type = http_content_type_text_html;
    }

    return (http_response_t) {
        .type = response_type,
        .content = content,
        .header = {
            .content_type = content_type,
            .connection_type = request->header.connection_type,
            .keep_alive_timeout_seconds = KEEP_ALIVE_TIMEOUT_SECONDS
        }
    };
}