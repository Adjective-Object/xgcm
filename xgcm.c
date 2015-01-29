#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include "xgcm_parser.h"
#include "xgcm_conf.h"
#include "ini/ini.h"

extern int optind;

char * CONFIG_FILE = "~/.config/xgcm/xgcmrc";

xgcm_configuration conf;

void print_helptext() {
    printf("helptext not writte :L\n");
}


static struct option o_verbose = {
    .name = "verbose",
    .has_arg = 0,
    .flag = NULL,
    .val = 'v'
};

static struct option o_symlink = {
    .name = "follow-symlinks",
    .has_arg = 0,
    .flag = NULL,
    .val = 'l'
};

static struct option o_recursive = {
    .name = "recursive",
    .has_arg = 0,
    .flag = NULL,
    .val = 'r'
};

static struct option o_config = {
    .name = "conf",
    .has_arg = 1,
    .flag = NULL,
    .val = 'c'
};

struct option long_opts[4];

void initialize_long_opts() {
    long_opts[0] = o_recursive;
    long_opts[1] = o_config;
    long_opts[2] = o_verbose;
    long_opts[3] = o_symlink;
}


void handle_option(char option) {
    switch(option) {
        default:
            print_helptext();
            exit(1);
    }
}




int main(int argc, char **argv) {
    initialize_long_opts();

    // load the configuration file, printing errors on failure
    if (ini_parse(CONFIG_FILE, handle_ini, &conf) < 0 ) {
        // no config file loaded
        build_default_config(&conf);
    }

    // read all options from getopt, overwriting the options specified
    // in the ini file
    char option;
    while ((option = getopt_long(argc, argv,"rvl:c:", long_opts, NULL)) != -1) {
        handle_option(option);
    }

    // if no targets specified, default to the targets loaded from
    // the config file. 
    if (optind == argc){
        char *path = next_path(&conf);
        // go to config if no path has been set
        if (path == NULL)
            path = "~/.config";

        for (;  path != NULL; 
                path = next_path(&conf)) {
            convert_by_path(path);
        }
    }
    // else, parse the remaining arguments as either directories to traverse
    // or as paths of files to read
    else {
        for ( ;optind < argc; optind++) {
            convert_by_path(argv[optind]);
        }
    }
    // exit successfully
    return 0;
}