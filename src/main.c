#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>

typedef int socket_t;
typedef struct sockaddr sockaddr_t;
typedef struct sockaddr_in sockaddr_in_t;

#define NUM_CONNECTIONS 256
#define MAX_NUM_REQUEST_MSG_CHARS 1024

int main() {
    socket_t listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == -1) {
        printf("Error creating socket: %s\n", strerror(errno));  
        return 1;
    }
    
    srand((uint32_t) time(NULL));
    uint16_t port = (uint16_t)(3000 + rand() % 32);
    printf("Port is %d\n", port);

    if (bind(listen_sock, (const sockaddr_t*) &(sockaddr_in_t) {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = { .s_addr = htonl(INADDR_ANY) }
    }, sizeof(sockaddr_in_t)) == -1) {
        printf("Error binding socket: %s\n", strerror(errno));
        return 1;
    }

    if (listen(listen_sock, NUM_CONNECTIONS) == -1) {
        printf("Error making socket listen: %s\n", strerror(errno));
        return 1;
    }
    
    
    sockaddr_in_t client_addr;
    socket_t client_sock = accept(listen_sock, (sockaddr_t*) &client_addr, (socklen_t[]) { sizeof(client_addr) });
    if (client_sock == -1) {
        printf("Error getting client sock: %s\n", strerror(errno));
        return 1;
    }

    char request_msg_chars[MAX_NUM_REQUEST_MSG_CHARS + 1];

    ssize_t num_request_msg_chars = recv(client_sock, request_msg_chars, MAX_NUM_REQUEST_MSG_CHARS, 0);
    if (num_request_msg_chars == -1) {
        printf("Error receiving data: %s\n", strerror(errno));
        return 1;
    }

    string_t request_msg = {
        .chars = request_msg_chars,
        .num_chars = (size_t)num_request_msg_chars
    };

    printf("Client request: %.*s\n", (int) request_msg.num_chars, request_msg.chars); 

    http_request_t request;
    if (parse_http_request_message(request_msg, &request) != result_success) {
        printf("Error parsing request\n");
    }

    printf("Request type: %d\n", request.type);
    for (size_t i = 0; i < request.num_headers; i++) {
        printf("Key: %.*s, Value: %.*s\n",
            (int) request.headers_key[i].num_chars,
            request.headers_key[i].chars,
            (int) request.headers_value[i].num_chars,
            request.headers_value[i].chars
        );
    }
    
    string_t content = MAKE_STRING("Hello world!");

    size_t num_content_length_str_chars = (size_t) snprintf(NULL, 0, "%lu", content.num_chars);
    char* content_length_str_chars = malloc(num_content_length_str_chars + 1);

    string_t content_length_str = {
        .num_chars = num_content_length_str_chars,
        .chars = content_length_str_chars
    };

    sprintf(content_length_str_chars, "%lu", content.num_chars);

    string_t response_headers_key[2] = {
        MAKE_STRING("Content-Length"),
        MAKE_STRING("Content-Type")
    };
    string_t response_headers_value[2] = {
        content_length_str,
        MAKE_STRING("text/plain")
    };

    http_response_t response = {
        .type = http_response_type_ok,
        .content = content,
        .num_headers = 2,
        .headers_key = response_headers_key,
        .headers_value = response_headers_value
    };

    string_t response_msg;
    create_http_response_message(&response, &response_msg);

    printf("Client response: %.*s\n", (int) response_msg.num_chars, response_msg.chars);

    send(client_sock, response_msg.chars, response_msg.num_chars, 0);
    
    close(client_sock);
    close(listen_sock);

    return 0;
}
