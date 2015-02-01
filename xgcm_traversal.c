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

	printf("\nrawpath: %s\n", rawpath);
	char * path = get_input_path(conf, rawpath);

	//check if file exists
	struct stat fstat;
	if ( 0 > lstat(path, &fstat) ) {
		df_printf( 
		"error: file %s does not exist\n", path);
		free(path);
	}
	else {

		// is symlink 
		if (S_IFLNK == (fstat.st_mode & S_IFMT) && conf->follow_symlinks) {
			d_printf("traversing symlink...\n");

			char * path_buf = 
				malloc(PATH_BUF_LEN);

			if (-1 != readlink(path, path_buf, PATH_BUF_LEN)) {
				df_printf( "error reading symlink %s\n", path);
				return;
			}

			convert_by_path(conf, path_buf);
			free(path_buf);
			return;
		}

		// is file
		else if (S_IFREG == (fstat.st_mode & S_IFMT)) {
			convert_file(conf, path);
			return;
		} 
		
		// is directory
		else if (S_IFDIR == (fstat.st_mode & S_IFMT)) {
			scan_directory(conf, path);
			return;
		}

		// unknown format
		else {
			df_printf( "unknown format, not parsing %s\n", path);
		}

	}
}

void scan_directory(xgcm_conf * conf, const char * path) {
	DIR *dp;
	struct dirent *ep;
	dp = opendir(path);

	if (dp != NULL) {
		while( (ep = readdir(dp)) ) {
			if (path_endswith(ep->d_name, conf->file_extension)) {
				convert_file(conf, ep->d_name);
			}

		}
	} else{
		df_printf( "cannot open directory %s", path);
	}
}


char * get_input_path(xgcm_conf * conf, const char * in_path) {
    if (path_endswith(in_path, conf->file_extension)) {
    	char * m = malloc(sizeof(char) * (strlen(in_path) + 1));
    	strcpy(m, in_path);
        return m;
    } else{
    	char * m = malloc(sizeof(char) * 
    		(strlen(in_path) + strlen(conf->file_extension) + 2));
    	strcpy(m, in_path);
    	strcat(m, ".");
    	strcat(m, conf->file_extension);
        return m;
    }
}


char * get_output_path(xgcm_conf * conf, const char * in_path) {
    if (path_endswith(in_path, conf->file_extension)) {
        return extless_path(in_path);
    } else{
    	char * m = malloc(sizeof(char) * (strlen(in_path) + 1));
    	strcpy(m, in_path);
        return m;
    }
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
    out_path[sublen+1] = '\0';

    return out_path;
}