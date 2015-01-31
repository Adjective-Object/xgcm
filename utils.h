#ifndef XGCM_UTILS
#define XGCM_UTILS

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "xgcm_conf.h"

int strbool(bool *b, const char * comp);

bool path_endswith(const char * string, const char * ext);
bool str_endswith(const char * string, const char * suffix);


typedef struct string_buffer {
    char * content;
    int len;
    int maxlen;
} sbuffer;

void buffer_init(sbuffer * b, int length);
bool buffer_putc(sbuffer * b, char c);
void buffer_clear(sbuffer * b);
void buffer_write(sbuffer * b, FILE *f);

char * strip_string_whitespace(const char * str);

#endif