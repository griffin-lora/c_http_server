#include "list.h"
#include <string.h>

void push_node_to_list(list_t* list, list_node_t* node) {
    if (list->head == NULL) {
        *node = (list_node_t) {
            .prev = NULL,
            .next = NULL
        };

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
    
    list->tail->next = node;

    list->tail = node;
}

void remove_node_from_list(list_t* list, const list_node_t* node) {
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
}