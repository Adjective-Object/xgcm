#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include "xgcm_parser.h"
#include "xgcm_conf.h"
#include "ini/ini.h"

#define d_printf(...) if (conf.verbose) {printf(__VA_ARGS__);}
#define df_printf(...) if (conf.verbose) {fprintf(stderr,__VA_ARGS__);}


extern int optind;

char * CONFIG_FILE = "~/.config/xgcm/xgcmrc";

void print_helptext() {
    printf("helptext not written :L\n");
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

static struct option o_ext = {
    .name = "file-ext",
    .has_arg = 1,
    .flag = NULL,
    .val = 'f'
};

struct option long_opts[4];

void initialize_long_opts() {
    long_opts[0] = o_recursive;
    long_opts[1] = o_config;
    long_opts[2] = o_verbose;
    long_opts[3] = o_symlink;
}


void handle_option(xgcm_conf *c, char option) {
    switch(option) {
        case 'v': // is verbose
            c->verbose = true;
            break;
        case 'l': // follow symlinks
            c->follow_symlinks = true;
            break;
        case 'r': // recursive directory search
            c->recursive = true;
            break;
        case 'c': // config
            CONFIG_FILE = optarg;
            break;
        case 'f': // file extensions
            c->file_extension = optarg;
            break;
        case 'i': // in place editing (instead of temp)
            c->make_temp_files = false;
        default: // WHOOPS
            print_helptext();
            exit(1);
    }
}




int main(int argc, char **argv) {
    initialize_long_opts();

    xgcm_configuration conf;
    build_default_config(&conf);

    // looks for '--conf in the options' 
    char option;
    while ((option = getopt_long(argc, argv,"rvl:c:", long_opts, NULL)) != -1) {
        if (option == 'c'){
            handle_option(&conf, option);
        }
    }

    // load the configuration file, printing errors on failure
    if (ini_parse(CONFIG_FILE, handle_ini, &conf) < 0 ) {
        df_printf("error loading the config file '%s'.\n", CONFIG_FILE);
    }

    // read remaining options from getopt, overwriting the config file's options
    optind = 1;
    while ((option = getopt_long(argc, argv,"rvl:c:", long_opts, NULL)) != -1) {
        handle_option(&conf, option);
    }



    // if no file targers specified, default to the targets loaded from
    // the config file. 
    if (optind == argc){
        char *path = next_path(&conf);
        // go to config if no path has been set
        if (path == NULL) {
            d_printf("no search targets specified, defaulting to '~/.config/'\n");
            add_file(&conf, "~/.config/");
        }


        for (; 
                path !=NULL; 
                path = next_path(&conf)) {
            convert_by_path(&conf, path);
        }
    }
    // else, parse the remaining arguments as either directories to traverse
    // or as paths of files to read
    else {
        for ( ;optind < argc; optind++) {
            d_printf("%s\n", argv[optind]);
            convert_by_path(&conf, argv[optind]);
        }
    }
    d_printf("\nfinished parsing\n");
    // exit successfully
    return 0;
}