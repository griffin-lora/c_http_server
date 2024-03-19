#include "server.h"
#include "http.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>

typedef int socket_t;
typedef struct sockaddr sockaddr_t;
typedef struct sockaddr_in sockaddr_in_t;

#define NUM_CONNECTIONS 256
#define MAX_NUM_REQUEST_MSG_CHARS 1024

static result_t handle_client_socket(socket_t sock) {
    char request_msg_chars[MAX_NUM_REQUEST_MSG_CHARS + 1];

    ssize_t num_request_msg_chars = recv(sock, request_msg_chars, MAX_NUM_REQUEST_MSG_CHARS, 0);
    if (num_request_msg_chars == -1) {
        return result_socket_failure;
    }

    string_t request_msg = {
        .chars = request_msg_chars,
        .num_chars = (size_t)num_request_msg_chars
    };

    printf("Client request: %.*s\n", (int) request_msg.num_chars, request_msg.chars); 

    http_request_t request;
    result_t result = parse_http_request_message(request_msg, &request);
    if (result != result_success) { return result; }

    printf("Request type: %d\n", request.type);
    
    http_response_t response = {
        .type = http_response_type_ok,
        .content = MAKE_STRING("Hello world!"),
        .header = {
            .content_type = http_content_type_text_plain,
            //.connection_type = request.header.connection_type
            .connection_type = http_connection_type_close
        }
    };

    string_t response_msg;
    create_http_response_message(&response, &response_msg);

    printf("Client response: %.*s\n", (int) response_msg.num_chars, response_msg.chars);

    send(sock, response_msg.chars, response_msg.num_chars, 0);
    
    close(sock);

    return result_success;
}

typedef union {
    socket_t socket;
    result_t result;
} client_thread_arg_t;

static void* client_thread_main(void* v_arg) {
    client_thread_arg_t* arg = v_arg;
    socket_t sock = arg->socket;
    arg->result = handle_client_socket(sock);
    return NULL;
}

result_t listen_for_clients(uint16_t port) {
    socket_t listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == -1) {
        return result_socket_failure;
    }

    if (bind(listen_sock, (const sockaddr_t*) &(sockaddr_in_t) {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = { .s_addr = htonl(INADDR_ANY) }
    }, sizeof(sockaddr_in_t)) == -1) {
        return result_socket_failure;
    }

    if (listen(listen_sock, NUM_CONNECTIONS) == -1) {
        return result_socket_failure;
    }
    
    sockaddr_in_t client_addr;
    socket_t client_sock = accept(listen_sock, (sockaddr_t*) &client_addr, (socklen_t[]) { sizeof(client_addr) });
    if (client_sock == -1) {
        return result_socket_failure;
    }

    client_thread_arg_t client_thread_arg = { .socket = client_sock };

    pthread_t client_thread;
    pthread_create(&client_thread, NULL, client_thread_main, &client_thread_arg);

    pthread_join(client_thread, NULL);
    if (client_thread_arg.result != result_success) {
        printf("Client thread failed\n");
        print_result_error(client_thread_arg.result);
    }

    close(listen_sock);

    return result_success;
}
