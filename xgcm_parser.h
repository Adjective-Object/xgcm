#ifndef XGCM_PARSER
#define XGCM_PARSER

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include "xgcm_conf.h"


int convert_file(xgcm_conf *conf, const char *path);

char *get_input_path(xgcm_conf *conf, const char *in_path);

char *get_output_path(xgcm_conf *conf, const char *in_path);

char *get_writing_path(xgcm_conf *conf, const char *in_path);

void mk_temp_dir(xgcm_conf *conf);

#define FBUFLEN 255
#define CAPTURE_BUF_LEN 1024
#define WRITE_BUF_LEN 1024
#define STARTING_TAG "{["
#define ENDING_TAG "]}"
#define TAG_LENGTH_MINUS_ONE 1
#define TAG_LENGTH 2


#endif