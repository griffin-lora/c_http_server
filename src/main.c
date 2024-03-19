#include "http.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>



int main() { 
    srand((uint32_t) time(NULL));
    uint16_t port = (uint16_t)(3000 + rand() % 32);
    printf("Listening on port %d\n", port);

    result_t result = listen_for_clients(port);
    if (result != result_success) {
        print_result_error(result);
    }
 
    return 0;
}
