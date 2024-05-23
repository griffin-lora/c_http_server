#include <stddef.h>
#include <stdint.h>

typedef struct list_node list_node_t;

struct list_node {
    list_node_t* prev;
    list_node_t* next;
    uint8_t data[];
};

typedef struct {
    list_node_t* head;
    list_node_t* tail;
} list_t;

void push_elem_to_list(list_t* list, size_t num_elem_bytes, const void* elem);
void remove_elem_from_list(list_t* list, const list_node_t* node);