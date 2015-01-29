#ifndef XGCM_CONF
#define XGCM_CONF
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	node * files;
	node * files_tail;
	hmap * relations;
	bool recursive;
	bool follow_symlinks;
	bool verbose;
} xgcm_configuration;

int handle_ini(
    void * void_conf, const char * section, 
    const char * name, const char * value);

void build_default_config(xgcm_configuration * conf);

char * next_path(xgcm_configuration * conf);
char * get_relation(xgcm_configuration * conf, const char * relation);

static void add_files(xgcm_configuration * conf, const char * files);
static void add_file(xgcm_configuration * conf, const char * value);
static void add_relation(xgcm_configuration * conf, 
		const char * key, const char * value);


#endif
