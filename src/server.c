#include "server.h"
#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <stdatomic.h>

typedef int socket_t;
typedef struct sockaddr sockaddr_t;
typedef struct sockaddr_in sockaddr_in_t;
typedef struct timeval timeval_t;

#define NUM_CONNECTIONS 256
#define MAX_NUM_REQUEST_MSG_CHARS 4096
#define KEEP_ALIVE_TIMEOUT_SECONDS 5

typedef struct {
    socket_t socket;
    pthread_t thread;
} client_thread_shutdown_info_t;

typedef struct client_thread_shutdown_info_node client_thread_shutdown_info_node_t;

typedef struct client_thread_shutdown_info_node {
    client_thread_shutdown_info_t info;
    client_thread_shutdown_info_node_t* prev;
    client_thread_shutdown_info_node_t* next;
} client_thread_shutdown_info_node_t;

static atomic_bool server_active = false;
static pthread_mutex_t client_thread_shutdown_info_list_mutex = PTHREAD_MUTEX_INITIALIZER;
static client_thread_shutdown_info_node_t* client_thread_shutdown_info_list_head = NULL;
static client_thread_shutdown_info_node_t* client_thread_shutdown_info_list_tail = NULL; 
static socket_t listen_socket;

static result_t handle_client_socket(socket_t sock, char* request_msg_chars) {
    while (server_active) {
        ssize_t num_request_msg_chars = recv(sock, request_msg_chars, MAX_NUM_REQUEST_MSG_CHARS, 0);
        if (num_request_msg_chars == -1) {
            if (errno == EAGAIN) {
                printf("Client socket timed out\n");
                return result_success;
            }
            return result_socket_failure;
        }

        string_t request_msg = {
            .chars = request_msg_chars,
            .num_chars = (size_t)num_request_msg_chars
        };

        printf("Client request (Thread 0x%lx):\n%.*s\n", pthread_self(), (int) request_msg.num_chars, request_msg.chars); 

        http_request_t request;
        result_t result = parse_http_request_message(request_msg, &request);
        if (result != result_success) { return result; }
 
        http_response_t response = {
            .type = http_response_type_ok,
            .content = MAKE_STRING("Hello world!"),
            .header = {
                .content_type = http_content_type_text_plain,
                .connection_type = request.header.connection_type,
                .keep_alive_timeout_seconds = KEEP_ALIVE_TIMEOUT_SECONDS
            }
        };

        string_t response_msg;
        create_http_response_message(&response, &response_msg);

        printf("Client response (Thread 0x%lx):\n%.*s\n", pthread_self(), (int) response_msg.num_chars, response_msg.chars);

        if (send(sock, response_msg.chars, response_msg.num_chars, 0) == -1) {
            return result_socket_failure;
        }

        free((char*)response_msg.chars);

        if (request.header.connection_type != http_connection_type_keep_alive) {
            break;
        }
    } 

    return result_success;
}

typedef struct {
    socket_t socket;
    client_thread_shutdown_info_node_t* node;
} client_thread_arg_t;

static void* client_thread_main(void* v_arg) {
    printf("Starting client thread (Thread 0x%lx)\n", pthread_self());

    client_thread_arg_t* arg = v_arg;
    socket_t sock = arg->socket;
    client_thread_shutdown_info_node_t* node = arg->node;
    free(arg);

    char* request_msg_chars = malloc(MAX_NUM_REQUEST_MSG_CHARS);

    result_t result = handle_client_socket(sock, request_msg_chars);

    free(request_msg_chars);
    
    // We don't care about error checking here as it does not matter, as long as the socket is closed successfully if it was open
    // Plus it could pollute an error from the handle_client_socket function
    close(sock);

    printf("Ending client thread (Thread 0x%lx)\n", pthread_self());
    
    // Only remove the client shutdown info node if the server is not trying to shut down
    if (server_active) {
        pthread_mutex_lock(&client_thread_shutdown_info_list_mutex);

        if (node->prev) {
            node->prev->next = node->next;
        } else {
            client_thread_shutdown_info_list_head = node->next; 
        }
        if (node->next) {
            node->next->prev = node->prev;
        } else {
            client_thread_shutdown_info_list_tail = node->prev;
        }

        free(node);

        pthread_mutex_unlock(&client_thread_shutdown_info_list_mutex);
    }

    if (result != result_success) {
        printf("Client failure, error: ");
        print_result_error(result);
    }

    return NULL;
}

static result_t handle_listen_socket(socket_t listen_sock) {
    listen_socket = listen_sock;
    server_active = true;

    while (server_active) {
        sockaddr_in_t client_addr; 
        socket_t client_sock = accept(listen_sock, (sockaddr_t*) &client_addr, (socklen_t[]) { sizeof(client_addr) }); 
        if (client_sock == -1) {
            return result_socket_failure;
        }

        if (setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (void*) &(timeval_t) {
            .tv_sec = KEEP_ALIVE_TIMEOUT_SECONDS 
        }, sizeof(timeval_t)) == -1) {
            return result_socket_failure;
        }
         
        pthread_mutex_lock(&client_thread_shutdown_info_list_mutex);

        client_thread_shutdown_info_node_t* node = malloc(sizeof(client_thread_shutdown_info_node_t));
        if (client_thread_shutdown_info_list_head == NULL) {
            node = malloc(sizeof(client_thread_shutdown_info_node_t));
            client_thread_shutdown_info_list_head = node;
        }
        *node = (client_thread_shutdown_info_node_t) {
            .info = {
                .socket = client_sock
            },
            .prev = client_thread_shutdown_info_list_tail,
            .next = NULL
        };
        if (client_thread_shutdown_info_list_tail) {
            client_thread_shutdown_info_list_tail->next = node;   
        } 
        client_thread_shutdown_info_list_tail = node;
        
        // Heap allocating this since we don't want to accidentally lose scope of the arguments.
        client_thread_arg_t* client_thread_arg = malloc(sizeof(client_thread_arg_t));
        *client_thread_arg = (client_thread_arg_t) {
            .socket = client_sock,
            .node = node
        };
        
        if (pthread_create(&node->info.thread, NULL, client_thread_main, client_thread_arg) != 0) {
            return result_thread_failure;
        }
 
        pthread_mutex_unlock(&client_thread_shutdown_info_list_mutex);
    }

    return result_success;
}

typedef union {
    socket_t socket;
    result_t result;
} listen_thread_arg_t;

static void* listen_thread_main(void* v_arg) {
    printf("Starting listen thread\n");

    listen_thread_arg_t* arg = v_arg;
    socket_t sock = arg->socket;

    arg->result = handle_listen_socket(sock);

    // Similar to before we do not care about error checking
    close(sock);

    printf("Ending listen thread\n");

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

    listen_thread_arg_t listen_thread_arg = { .socket = listen_sock };
    
    pthread_t listen_thread; 
    if (pthread_create(&listen_thread, NULL, listen_thread_main, &listen_thread_arg) != 0) {
        return result_thread_failure;
    }

    if (pthread_join(listen_thread, NULL) != 0) {
        return result_thread_failure;
    } 

    if (listen_thread_arg.result != result_success) {
        return listen_thread_arg.result;
    }

    return result_success;
}

void shutdown_server(void) {
    if (!server_active) {
        return;
    }
    server_active = false;

    pthread_mutex_lock(&client_thread_shutdown_info_list_mutex);

    for (client_thread_shutdown_info_node_t* node = client_thread_shutdown_info_list_head; node != NULL; node = node->next) {
        if (shutdown(node->info.socket, 0) == -1) {
            print_result_error(result_socket_failure);
        }

        if (pthread_join(node->info.thread, NULL) != 0) {
            print_result_error(result_thread_failure);
        }
    }

    pthread_mutex_unlock(&client_thread_shutdown_info_list_mutex);

    if (shutdown(listen_socket, 0) == -1) {
        print_result_error(result_socket_failure);
    }
}
