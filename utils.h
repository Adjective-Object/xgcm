#ifndef XGCM_UTILS
#define XGCM_UTILS

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include "xgcm_conf.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h> 

#define d_pdepth(stream) if (conf->verbose) {pdepth(stream);}
#define d_printf(...) if (conf->verbose) {printf(__VA_ARGS__);}
#define df_printf(...) if (conf->verbose) {fprintf(stderr,__VA_ARGS__);}

#define perrorf(...) \
    char * errmsg = malloc(200); \
    sprintf(errmsg, __VA_ARGS__); \
    perror(errmsg); \
    free(errmsg);


int TABS;

void pdepth(FILE *fd);

void tabup();

void tabdown();

bool strbool(bool *b, const char *comp);

bool path_endswith(const char *string, const char *ext);

bool str_endswith(const char *string, const char *suffix);

char *extless_path(const char *in_path);

char *strip_string_whitespace(const char *str);

const char *chdir_to_parent(const char *rawpath);

char * expand_path(char *rawpath);

void lua_stackDump (lua_State *L);
void lua_globalDump (lua_State *L);


#endif
