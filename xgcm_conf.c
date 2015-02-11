#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wordexp.h>
#include "ini/ini.h"
#include "simple_hmap.h"
#include "utils.h"
#include "xgcm_conf.h"



bool relations_header = true;
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
        if (relations_header) {
            d_printf("relations:\n");
            relations_header = false;
        }
        d_printf("  ~ '%s': '%s'\n",
                name, value);
        add_relation(conf, name, value);
    }
    else { 
        df_printf( "failed to match section='%s' value='%s'\n",
                    section, name);
        return 0;
    }
    return 1;
}

void build_default_config(xgcm_configuration * conf){
    conf->recursive = true;
    conf->follow_symlinks = false;
    conf->verbose = false;
    conf->make_temp_files = true;

    conf->files = NULL;
    conf->files_tail = NULL;

    conf->relations = malloc(sizeof(hmap));
    hmap_init(conf->relations, 50);

    char * deftemp = "/tmp/xgcm/";
    conf->tempdir_path = malloc(sizeof(char) * (strlen(deftemp) + 1));
    strcpy(conf->tempdir_path, deftemp);

    char * defext = "xgcm";
    conf->file_extension = malloc(sizeof(char) * (strlen(defext) + 1));
    strcpy(conf->file_extension, defext);
}



void add_files(xgcm_configuration * conf, const char * files) {
    printf("files:\n");

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

    d_printf ("  + '%s'\n",
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
    hmap_append_str (conf->relations, key, value, strlen(value) + 1, ' ');
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

void print_conf(xgcm_configuration * conf, char * context) {
    printf("xgcm_conf '%s': {\n", context);
    printf("        version: %d\n", conf->version);
    
    printf("      recursive: %d\n", conf->recursive);
    printf("follow_symlinks: %d\n", conf->follow_symlinks);
    printf("        verbose: %d\n", conf->verbose);
    printf("make_temp_files: %d\n", conf->make_temp_files);

    printf("   tempdir_path: '%s'\n", conf->tempdir_path);
    printf(" file_extension: '%s'\n", conf->file_extension);
    printf("}\n");
}