#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include "string_buffer.h"

void buffer_init(sbuffer * b, int maxlen) {
    b->content = malloc(sizeof(char) * (maxlen + 1));
    b->content[0] = '\0';
    b->maxlen = maxlen;
    b->len = 0;
}

void buffer_teardown(sbuffer * b) {
    free(b->content);
}

bool buffer_putc(sbuffer * b, char c){
    if (b->len >= b->maxlen)
        return false;
    b->content[b->len] = c;
    b->len++;
    b->content[b->len] = '\0';
    return true;
}

void buffer_write(sbuffer * b, FILE * f){  
    fwrite(b->content,
            sizeof(char),
            b->len,
            f);
}

void buffer_clear(sbuffer * b){
    b->content[0] = '\0';
    b->len = 0;
}