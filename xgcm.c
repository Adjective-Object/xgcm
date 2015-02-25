#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include "xgcm_traversal.h"
#include "xgcm_conf.h"


#define d_printf(...) if (conf.verbose) {printf(__VA_ARGS__);}
#define df_printf(...) if (conf.verbose) {fprintf(stderr,__VA_ARGS__);}

extern int optind;

char *CONFIG_FILES_RAW = "~/.config/xgcm/xgcmrc";

void print_helptext() {
    printf("helptext not written :L\n");
}


const struct option long_opts[5] = {
        {
                .name = "verbose",
                .has_arg = 0,
                .flag = NULL,
                .val = 'v'
        },
        {
                .name = "follow-symlinks",
                .has_arg = 0,
                .flag = NULL,
                .val = 'l'
        },
        {
                .name = "recursive",
                .has_arg = 0,
                .flag = NULL,
                .val = 'r'
        },
        {
                .name = "conf",
                .has_arg = 1,
                .flag = NULL,
                .val = 'c'
        },
        {
                .name = "file-ext",
                .has_arg = 1,
                .flag = NULL,
                .val = 'f'
        }
};


void handle_option(xgcm_conf *c, int option) {
    switch (option) {
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
            CONFIG_FILES_RAW = optarg;
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

    xgcm_configuration conf;
    build_default_config(&conf);

    // looks for '--conf in the options'
    int option;
    while ((option = getopt_long(argc, argv, "rvl:c:", long_opts, NULL)) != -1) {
        if (option == 'c' || option == 'v') {
            handle_option(&conf, option);
        }
    }

    // traverse the linked list of config files in reverse order, parsing all
    conf_init();

    ll *cfiles = ll_init();
    ll_from_string(cfiles, CONFIG_FILES_RAW, ';');

    printf("config files:\n");
    while (cfiles->head != NULL) {
        char *rawpath = ll_pop_head(cfiles);
        printf("rawpath = %p\n", (void *) rawpath);
        enqueue_conf_file(rawpath);
    }
    parse_conf_files(&conf);


    // read remaining options from getopt, overwriting the config file's options
    optind = 1;
    while ((option = getopt_long(argc, argv, "rvl:c:", long_opts, NULL)) != -1) {
        handle_option(&conf, option);
    }


    if (conf.verbose) {
        print_conf(&conf, "init");
        printf("\nfound relations: \n");
        hmap_disp(conf.relations);
        printf("\n");
    }


    // if no file targers specified, default to the targets loaded from
    // the config file.
    if (optind == argc) {
        char *path = next_path(&conf);
        // go to config if no path has been set
        if (path == NULL) {
            d_printf("no search targets specified, defaulting to '~/.config/'\n");
            add_files(&conf, "~/.config/");
        }


        for (;
                path != NULL;
                path = next_path(&conf)) {
            printf("==%s\n", path);
            convert_by_path(&conf, path);
            free(path);
        }
    }
        // else, parse the remaining arguments as either directories to traverse
        // or as paths of files to read
    else {
        for (; optind < argc; optind++) {
            printf("==%s\n", argv[optind]);
            convert_by_path(&conf, argv[optind]);
        }
    }
    d_printf("\nfinished parsing\n");
    teardown_config(&conf);
    // exit successfully
    return 0;
}