#pragma once
#include "result.h"
#include <stdint.h>

result_t listen_for_clients(uint16_t port);
void shutdown_server(void);
