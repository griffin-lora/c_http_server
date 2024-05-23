#include "list.h"
#include <stdlib.h>
#include <string.h>

void push_elem_to_list(list_t* list, size_t num_elem_bytes, const void* elem) {
    list_node_t* node = malloc(sizeof(*node) + num_elem_bytes);

    if (list->head == NULL) {
        *node = (list_node_t) {
            .prev = NULL,
            .next = NULL
        };
        memcpy(node->data, elem, num_elem_bytes);

        *list = (list_t) {
            .head = node,
            .tail = node
        };
        return;
    }

    *node = (list_node_t) {
        .prev = list->tail,
        .next = NULL
    };
    memcpy(node->data, elem, num_elem_bytes);
    
    list->tail->next = node;

    list->tail = node;
}

void remove_elem_from_list(list_t* list, const list_node_t* node) {
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        list->head = node->next;
    }
    
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        list->tail = node->prev;
    }

    free((void*) node);
}