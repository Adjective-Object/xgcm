#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "xgcm_parser.h"
#include <sys/stat.h>


extern bool FOLLOW_SYMLINKS;
extern char * FILE_EXT;
extern bool RECURSIVE;

#define PATH_BUF_LEN = sizeof(char) * 1000;

void convert_by_path(const char* path) {

	//check if file exists
	struct stat fstat;
	if ( !stat(path, &fstat) ) {
		fprintf(stder, 
		"error: specified file %*s does not exist",
		strlen(path),
		path);
	}
	else {

		// is file
		if (S_ISREG(fstat.st_mode)) {
			convert_file(path);
		} 
		
		// is directory
		if (S_ISDIR(fstat.st_mode)) {
			scan_directory(path);
		}

		// is symlink 
		if (FOLLOW_SYMLINKS 
			&& S_IFLNK(fstat.st_mode)) {

			char * path_buf = 
				malloc(PATH_BUF_LEN);

			if (!readlink(path, &path_buf, PATH_BUF_LEN)) {
				
			}
			
			convert_by_path(path_buf);
			free(path_buf);

		}

	}

}
