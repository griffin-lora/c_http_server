#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

typedef int socket_t;
typedef struct sockaddr sockaddr_t;
typedef struct sockaddr_in sockaddr_in_t;

#define NUM_CONNECTIONS 256

int main() {
    socket_t listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == -1) {
        printf("Error creating socket: %s\n", strerror(errno));  
        return 1;
    }

    if (bind(listen_sock, (const sockaddr_t*) &(sockaddr_in_t) {
        .sin_family = AF_INET,
        .sin_port = htons(3000),
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
    
    char client_msg[256];
    memset(client_msg, 0, 256);

    ssize_t num_bytes = recv(client_sock, client_msg, 255, MSG_WAITALL);
    if (num_bytes == -1) {
        printf("Error receiving data: %s\n", strerror(errno));
        return 1;
    }

    printf("Client sent:\n%s\n", client_msg);
    
    close(client_sock);
    close(listen_sock);

    return 0;
}
