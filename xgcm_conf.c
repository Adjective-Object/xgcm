#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ini/ini.h"
#include "simple_hmap.h"
#include "utils.h"
#include "xgcm_conf.h"

int handle_ini(
    void * void_conf, const 
    char * section, const char * name, const char * value) {

    xgcm_configuration* conf = (xgcm_configuration*)void_conf;

    if (MATCH("xgcm", "version")) {
        conf->version = atoi(value);
    }
    else if (MATCH("xgcm", "files")) {
        add_files(conf, value);
    }
    else if (MATCH("xgcm", "recursive")) {
        return strbool(&(conf->recursive),value);
    }
    else if (MATCH("xgcm", "follow_symlinks")) {
        return strbool(&(conf->follow_symlinks),value);
    }
    else if (MATCH("xgcm", "verbose")) {
        return strbool(&(conf->verbose),value);
    }
    else if (strcmp("attributes",section)) {
        add_relation(conf, name, value);
    }
    else { 
        return 0;
    }
    return 1;
}


void build_default_config(xgcm_configuration * conf){
    conf->recursive = true;
    conf->follow_symlinks = false;
    conf->verbose = true;
}



static void add_files(xgcm_configuration * conf, const char * files) {
    //TODO take a comma separated list of paths and insert them all
    char * buffer = malloc(strlen(files) + 1);
    memcpy(buffer, files, strlen(files) + 1);
    char * head = buffer;
    char * tail = buffer;
    int i, len = strlen(files) + 1;
    for(i=0; i<len; i++) {
        head ++;
        if (*head == ';' && tail) {
            if (head != tail) {
                *head = '\0';
                add_file(conf, tail);
            }
            tail = (char *)(head +1);
        }
    }
}

static void add_file(xgcm_configuration * conf, const char * value) {
    node * new_node = hmap_init_node(value, NULL, 0);
    conf->files_tail->next = new_node;
    conf->files_tail = new_node;
}

static void add_relation(xgcm_configuration * conf, const char * key, const char * value) {
    hmap_insert (conf->relations, key, value, strlen(value) + 1);
}

char * next_path(xgcm_configuration * conf) {
    if (conf->files == NULL) {
        return NULL;
    }
    char * name = conf->files->key;
    node * old_node = conf->files;
    conf->files = conf->files->next;
    free(old_node);
    return name;
}

char * get_relation(xgcm_configuration * conf, const char * key) {
    return hmap_lookup(conf->relations, key);
}