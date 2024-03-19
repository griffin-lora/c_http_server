#pragma once

typedef enum {
    result_success,
    result_failure,
    result_invalid_http_request_message,
    result_invalid_first_line,
    result_invalid_connection_type,
    result_invalid_header_line,
    result_socket_failure
} result_t;

void print_result_error(result_t result);
