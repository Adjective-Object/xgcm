#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wordexp.h>
#include "ini/ini.h"
#include "simple_hmap.h"
#include "utils.h"
#include "xgcm_conf.h"




int handle_ini(
    void * void_conf, const char * section, 
    const char * name, const char * value) {

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
    else if (0 == strcmp("attributes", section)) {
        printf("adding relation key='%s' value='%s'\n",
                name, value);
        add_relation(conf, name, value);
    }
    else { 
        fprintf(stderr, "failed to match section='%s' value='%s'\n",
                    section, name);
        return 0;
    }
    return 1;
}


void build_default_config(xgcm_configuration * conf){
    conf->recursive = true;
    conf->follow_symlinks = false;
    conf->verbose = true;

    conf->files = NULL;
    conf->files_tail = NULL;

    conf->relations = malloc(sizeof(hmap));
    hmap_init(conf->relations, 50);
}



void add_files(xgcm_configuration * conf, const char * files) {
    printf("adding files from conf..\n");

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
    add_file(conf, tail);   
}

void add_file(xgcm_configuration * conf, const char * rawpath) {

    wordexp_t expand;
    wordexp(rawpath, &expand, 0);
    char * path = expand.we_wordv[0];

    printf ("adding file '%s' to search path.\n",
            path);

    node * new_node = hmap_init_node(path, NULL, 0);
    if (conf->files_tail) {
        conf->files_tail->next = new_node;
        conf->files_tail = new_node;
    } else{
        conf->files = new_node;
        conf->files_tail = new_node;
    }

    wordfree(&expand);
    
}

void print_files(node * head){
    printf("[");
    while (head) {
        printf("%s, ",head->key);
        head = head->next;
    }
    printf("]\n");

}

void add_relation(
        xgcm_configuration * conf, 
        const char * key, const char * value) {
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
    char * stripped_key = strip_string_whitespace(key);
    char * result = hmap_lookup(conf->relations, stripped_key);
    free (stripped_key);
    return result;
}