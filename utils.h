#ifndef XGCM_UTILS
#define XGCM_UTILS

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "xgcm_conf.h"

#define d_printf(...) if (conf->verbose) {printf(__VA_ARGS__);}
#define df_printf(...) if (conf->verbose) {fprintf(stderr,__VA_ARGS__);}

int strbool(bool *b, const char * comp);

bool path_endswith(const char * string, const char * ext);
bool str_endswith(const char * string, const char * suffix);

char * strip_string_whitespace(const char * str);

#endif