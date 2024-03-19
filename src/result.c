#include "result.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>

static const char* get_result_string(result_t result) {
    switch (result) {
        case result_failure: return "Generic failure";
        case result_invalid_http_request_message: return "Invalid http request";
        case result_invalid_first_line: return "Invalid http request first line";
        case result_invalid_connection_type: return "Invalid http request connection type";
        case result_invalid_header_line: return "Invalid http request header line";
        default: return NULL;
    }
}

void print_result_error(result_t result) {
    if (result == result_success) {
        return;
    }
    if (result == result_socket_failure) {
        printf("Socket failure due to: %s\n", strerror(errno));
        return;
    }
    printf("%s\n", get_result_string(result));
}
