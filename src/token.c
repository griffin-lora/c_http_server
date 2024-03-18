#include "token.h"
#include <stdio.h>
#include <string.h>

static size_t get_next_start_index(const lexer_info_t* info, size_t token_index) {
    size_t delim_index = 0;

    for (size_t i = token_index; i < info->num_chars; i++) {
        if (info->str[i] != info->delim[delim_index]) {
            return i;
        }
        delim_index++;
        if (info->delim[delim_index] == '\0') {
            delim_index = 0;
        }
    }
    // We should never ever reach here so print an error for debug
    // printf("WARN: Reached end of get_next_start_index, probably should close this client socket\n");
    // return info->num_chars;
}

static size_t get_num_token_chars(const lexer_info_t* info, size_t token_index) {
    size_t num_token_chars = 0;
    size_t delim_index = 0;

    for (size_t i = token_index; i < info->num_chars; i++) {
        if (info->str[i] != info->delim[delim_index]) { 
            delim_index = 0;
            continue;
        }
        if (delim_index == 0) {
            num_token_chars = i - token_index;
        } 
        delim_index++;
        if (info->delim[delim_index] == '\0') {
            return num_token_chars;
        }
    }
    return info->num_chars - token_index;
}

lexer_t init_lexer(const lexer_info_t* info) {
    size_t token_index = get_next_start_index(info, 0);
    return (lexer_t) {
        .index = token_index,
        .num_chars = get_num_token_chars(info, token_index)
    };
}

bool check_lexer(const lexer_info_t* info, const lexer_t* lexer) {
    return lexer->index < info->num_chars;
}

lexer_t next_lexer(const lexer_info_t* info, const lexer_t* lexer) {
    size_t token_index = get_next_start_index(info, lexer->index + lexer->num_chars);
    return (lexer_t) {
        .index = token_index,
        .num_chars = get_num_token_chars(info, token_index)
    };
}
