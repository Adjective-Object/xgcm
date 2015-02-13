#ifndef SIMPLE_STRING_BUFFER
#define SIMPLE_STRING_BUFFER

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

typedef struct string_buffer {
    char * content;
    int len;
    int maxlen;
} string_buffer;
typedef string_buffer sbuffer;

void buffer_init(sbuffer * b, int length);
bool buffer_putc(sbuffer * b, char c);
void buffer_clear(sbuffer * b);
void buffer_write(sbuffer * b, FILE *f);
void buffer_teardown(sbuffer * b);

#endif