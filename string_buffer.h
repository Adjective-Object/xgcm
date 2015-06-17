#ifndef SIMPLE_STRING_BUFFER
#define SIMPLE_STRING_BUFFER

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

typedef struct string_buffer {
    char *content;
    size_t len;
    size_t maxlen;
} string_buffer;
typedef string_buffer sbuffer;

void buffer_init(sbuffer *b, size_t length);

bool buffer_putc(sbuffer *b, char c);

void buffer_clear(sbuffer *b);

void buffer_write(sbuffer *b, FILE *f);

void buffer_teardown(sbuffer *b);



typedef struct cyclic_string_buffer {
    char *content;
    char *writehead;
    size_t length;
    int count;
} cyclic_string_buffer;
typedef cyclic_string_buffer csbuffer;

void csbuffer_init(csbuffer *b, size_t maxlen);

char csbuffer_cycle(csbuffer *b, char c);

void csbuffer_clear(csbuffer *b);

#endif
