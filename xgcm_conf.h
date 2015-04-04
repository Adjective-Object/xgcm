#ifndef XGCM_CONF
#define XGCM_CONF

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h> 

#include "ini.h"
#include "simple_ll.h"

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

typedef struct __attribute__((__packed__)) string_node {
    struct string_node *next;
    char *val;
} s_node;

typedef struct __attribute__((__packed__)) relation_node {
    struct relation_node *next;
    char *key;
    char *val;
} r_node;


typedef struct xgcm_configuration {
    int version;

    bool recursive;
    bool explore_hidden;
    bool follow_symlinks;
    bool verbose;
    bool make_temp_files;

    char *tempdir_path;
    char *tempfile_prefix;
    char *file_extension;
    char *multiline_divider;

    lua_State *lua_state;

    ll *files;
} xgcm_configuration;
typedef xgcm_configuration xgcm_conf;


void conf_init();
void enqueue_conf_file(const char *rawpath);
void parse_conf_files(xgcm_conf *conf);

int handle_ini(
        void *void_conf, const char *section,
        const char *name, const char *value);

void build_default_config(xgcm_configuration *conf);

void load_paths_from_conf(xgcm_configuration *conf);


char *next_path(xgcm_configuration *conf);
char *get_relation(xgcm_configuration *conf, const char *relation);
char *lua_eval_return(xgcm_configuration *conf, const char *luaCall);


void add_files(xgcm_configuration *conf, const char *files);

void add_file(xgcm_configuration *conf, const char *value);

void expand_file(node *to_expand);
void add_relation(xgcm_configuration *conf,
        const char *key, const char *value);

void print_conf(xgcm_configuration *conf, char *context);

#endif
