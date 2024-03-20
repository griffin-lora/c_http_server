#pragma once
#include "result.h"
#include <stdint.h>
#include <stdatomic.h>

extern atomic_bool server_active; 

result_t listen_for_clients(uint16_t port);
void shutdown_server(void);
