#ifndef XGCM_PARSER
#define XGCM_PARSER

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include "xgcm_conf.h"
#include "string_buffer.h"

typedef struct parse_state {
    FILE * raw_file;
    FILE * temp_file;
    sbuffer * capture_buffer;
    sbuffer * write_buffer;
    csbuffer * tag_buffer;
    char * read_buffer;
    size_t readsize;
    bool capturing;
    int n_lines;
    int i;
    const char * path;
} parse_state;

int convert_file(xgcm_conf *conf, const char *path);

char *get_input_path(xgcm_conf *conf, const char *in_path);

char *get_final_path(xgcm_conf *conf, const char *in_path);

char *get_temp_path(xgcm_conf *conf, const char *in_path);

void mk_temp_dir(xgcm_conf *conf);

#define FBUFLEN 255
#define CAPTURE_BUF_LEN 1024
#define WRITE_BUF_LEN 1024
#define STARTING_TAG "{["
#define ENDING_TAG "]}"
#define TAG_LENGTH 2 * sizeof(char)


#endif
