#include <stddef.h>
#include <stdint.h>

typedef struct list_node list_node_t;

typedef struct {
    list_node_t* prev;
    list_node_t* next;
} list_node_header_t;

struct list_node {
    list_node_header_t header;
    uint8_t data[];
};

typedef struct {
    list_node_t* head;
    list_node_t* tail;
} list_t;

void push_node_to_list(list_t* list, list_node_t* node);
void remove_node_from_list(list_t* list, const list_node_t* node);