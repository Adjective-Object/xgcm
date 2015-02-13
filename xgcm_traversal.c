#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "xgcm_conf.h"
#include "xgcm_parser.h"
#include "xgcm_traversal.h"
#include "utils.h"
#include <sys/stat.h>

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>


void convert_by_path(xgcm_conf * conf, const char * rawpath) {
	pdepth(stdout);
	printf("> %s\n", rawpath);


	char * path = get_input_path(conf, rawpath);

	//check if file exists
	struct stat file_stat;
	if ( 0 > lstat(path, &file_stat) ) {
		fprintf(
			stderr, 
			"file %s does not exist, skipping\n", path);
		free(path);
	}
	else {
		// is symlink 
		if (S_IFLNK == (file_stat.st_mode & S_IFMT) && conf->follow_symlinks) {
			d_printf("traversing symlink...\n");

			char * path_buf = 
				malloc(PATH_BUF_LEN);

			if (readlink(path, path_buf, PATH_BUF_LEN)) {
				df_printf( "error reading symlink %s\n", path);
				return;
			}

			if (convert_file(conf, path_buf)){
				fprintf(stderr, "error parsing file '%s'\n", path_buf);
			}
			free(path_buf);
			return;
		}

		// is file
		else if (S_IFREG == (file_stat.st_mode & S_IFMT)) {
			d_pdepth(stdout);
			d_printf("    converting..\n");
			if (convert_file(conf, path)) {
				fprintf(stderr, "error parsing file '%s'\n", path);
			}
			return;
		} 
		
		// is directory
		else if (S_IFDIR == (file_stat.st_mode & S_IFMT)) {
			tabup();
			scan_directory(conf, path);
			tabdown();
			return;
		}

		// unknown format
		else {
			df_printf( "unknown format, not parsing %s\n", path);
		}

	}
}

void scan_directory(xgcm_conf * conf, const char * path) {
	struct dirent *ep;
	DIR *dp = opendir(path);

	if (dp != NULL) {
		struct stat buf;
		while( (ep = readdir(dp)) ) {
			char * newpath = malloc(strlen(ep->d_name) + strlen(path) + 2);
			strcpy(newpath, path);
			if(path[strlen(path) - 1] != '/'){
				strcat(newpath, "/");
			}
			strcat(newpath, ep->d_name);

			if (0 > stat(newpath, &buf) ) {
				df_printf("error getting stat of %s (%s)\n", 
					ep->d_name,
					newpath);
			}
			else if (S_IFREG == (buf.st_mode & S_IFMT)) {
				if (path_endswith(ep->d_name, conf->file_extension)) {
					convert_by_path(conf, newpath);
				}
			} else if (
					S_IFDIR == (buf.st_mode & S_IFMT) &&
					conf->recursive &&
					(conf->explore_hidden || *ep->d_name != '.') &&
					0 != strcmp(ep->d_name, "..") && 
					0 != strcmp(ep->d_name, ".")) {
				convert_by_path(conf, newpath);
			}

			free(newpath);
		}
	} else{
		df_printf( "cannot open directory %s", path);
	}
	closedir(dp);
}

char * extless_path(const char * original_path) {

    int len = strlen(original_path) + 1;

    // find the location of the point
    const char * last_dot = NULL;
    const char * tracer = original_path;
    while (*tracer != '\0') {
        if (*tracer =='.')
            last_dot = tracer;
        tracer++;
    }

    // if there was no period, set it to the end of the string
    if (last_dot == NULL)
        last_dot = original_path + len;

    int sublen = (int)(last_dot - original_path);
    char * out_path = malloc(sublen);
    memcpy(out_path, original_path, sublen);
    out_path[sublen] = '\0';

    return out_path;
}