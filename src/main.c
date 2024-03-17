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
    
    char request_msg[MAX_NUM_REQUEST_MSG_CHARS + 1];
    // memset(request_msg, 0, MAX_NUM_REQUEST_MSG_CHARS + 1);
    
    // TODO: While loop read in entire request
    ssize_t num_request_msg_chars = recv(client_sock, request_msg, MAX_NUM_REQUEST_MSG_CHARS, 0);
    if (num_request_msg_chars == -1) {
        printf("Error receiving data: %s\n", strerror(errno));
        return 1;
    }

    request_msg[num_request_msg_chars] = '\0';

    printf("Client request:\n%s\n", request_msg);

    http_request_t request;
    if (parse_http_request_message(request_msg, &request) != result_success) {
        printf("Error parsing request\n");
    }

    printf("%d\n", request.type);
    
    close(client_sock);
    close(listen_sock);

    return 0;
}
