#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "string_buffer.h"

void buffer_init(sbuffer *b, size_t maxlen) {
    b->content = malloc(sizeof(char) * (maxlen + 1));
    b->content[0] = '\0';
    b->maxlen = maxlen;
    b->len = 0;
}

void buffer_teardown(sbuffer *b) {
    free(b->content);
}

bool buffer_putc(sbuffer *b, char c) {
    if (b->len >= b->maxlen)
        return false;
    b->content[b->len] = c;
    b->len++;
    b->content[b->len] = '\0';
    return true;
}

void buffer_write(sbuffer *b, FILE *f) {
    fwrite(b->content,
            sizeof(char),
            b->len,
            f);
}

void buffer_clear(sbuffer *b) {
    b->content[0] = '\0';
    b->len = 0;
}

void csbuffer_init(csbuffer *b, size_t maxlen) {
    b->content = malloc(sizeof(char) * (maxlen));
    memset(b->content, '\0', sizeof(char) * (maxlen));
    b->writehead = b->content;
    b->length = maxlen;
    b->count = 0;
    // printf ("created buffer %p %d %d\n", b->content, b->length, maxlen);
}

char csbuffer_cycle(csbuffer *b, char c) {
    char toret = *(b->writehead);
    *(b->writehead) = c;
    b->writehead ++;
    b->count ++;
    if (b->writehead >= b->content + b->length)
        b->writehead = b->content;

    return toret;
}

void csbuffer_clear(csbuffer *b) {
    // printf("clearing: buffer, index %d, contents \"%.*s\"\n", 
    //     b->writehead - b->content,
    //     b->length,
    //     b->content);
    memset(b->content, 0, sizeof(char) * b->length);
    b->writehead = b->content;
    b->count = 0;
}
