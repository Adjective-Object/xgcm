#ifndef XGCM_CONF
#define XGCM_CONF
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ini/ini.h"
#include "simple_hmap.h"

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

typedef struct __attribute__((__packed__)) string_node {
	struct string_node * next;
	char * val;
} s_node;

typedef struct __attribute__((__packed__)) relation_node {
	struct relation_node * next;
	char * key;
	char * val;
} r_node;


typedef struct  xgcm_configuration {
	int version;

	bool recursive;
	bool explore_hidden;
	bool follow_symlinks;
	bool verbose;
	bool make_temp_files;
	
	char * tempdir_path;
	char * tempfile_prefix;
	char * file_extension;
	char * multiline_divider;
	
	node * files;
	node * files_tail;
	hmap * relations;
} xgcm_configuration;
typedef xgcm_configuration xgcm_conf;


int handle_ini(
    void * void_conf, const char * section, 
    const char * name, const char * value);

void build_default_config(xgcm_configuration * conf);
void load_paths_from_conf(xgcm_configuration *conf);

char * next_path(xgcm_configuration * conf);
char * get_relation(xgcm_configuration * conf, const char * relation);

void add_files(xgcm_configuration * conf, const char * files);
void add_file(xgcm_configuration * conf, const char * value);
void add_relation(xgcm_configuration * conf, 
        const char * key, const char * value);

void print_conf(xgcm_configuration * conf, char * context);

#endif
