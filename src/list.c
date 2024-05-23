#include "list.h"
#include <string.h>

void push_node_to_list(list_t* list, list_node_t* node) {
    if (list->head == NULL) {
        node->header = (list_node_header_t) {
            .prev = NULL,
            .next = NULL
        };

        *list = (list_t) {
            .head = node,
            .tail = node
        };
        return;
    }

    node->header = (list_node_header_t) {
        .prev = list->tail,
        .next = NULL
    };
    
    list->tail->header.next = node;

    list->tail = node;
}

void remove_node_from_list(list_t* list, const list_node_t* node) {
    if (node->header.prev) {
        node->header.prev->header.next = node->header.next;
    } else {
        list->head = node->header.next;
    }
    
    if (node->header.next) {
        node->header.next->header.prev = node->header.prev;
    } else {
        list->tail = node->header.prev;
    }
}