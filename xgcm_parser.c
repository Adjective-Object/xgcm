#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "xgcm_parser.h"
#include "xgcm_conf.h"
#include "utils.h"
#include <sys/stat.h>

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

#define PATH_BUF_LEN (sizeof(char) * 1000)




void convert_by_path(xgcm_conf * conf, const char * path) {

	printf("\npath: %s\n", path);
	//check if file exists
	struct stat fstat;
	if ( 0 > lstat(path, &fstat) ) {
		df_printf( 
		"error: specified file %s does not exist\n", path);
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


static void convert_file(xgcm_conf * conf, const char * path) {
	//TODO this
	d_printf("  processing file '%s' ", path);
	fflush(stdout);
	char * output_path = get_output_path(conf, path);
	d_printf("-> '%s'\n", output_path);

	FILE * raw_file = fopen(path, "r");
	FILE * out_file = fopen(output_path, "w");

	bool capturing = false;

	// solve the 'split tamplate tags' problem by prepending some number of 
	// characters from the previous read to the current one
	char read_buffer[FBUFLEN];
	int i, readsize, n_lines = 0;
	
	sbuffer capture_buffer, write_buffer;
	buffer_init(&capture_buffer, CAPTURE_BUF_LEN);
	buffer_init(&write_buffer, WRITE_BUF_LEN);

	while (0 < (readsize = fread(read_buffer, 
									sizeof(char), 
									FBUFLEN - TAG_LENGTH_MINUS_ONE, 
									raw_file))) {
		for (i=1; i<readsize; i++) {
			// parse the text at the current position. Assumes either tag
			// does not begin with a newline. Generally a safe assumption
			if (read_buffer[i] == '\n') {
				n_lines++;
			}
			if (0 == memcmp(&read_buffer[i], STARTING_TAG, TAG_LENGTH)) {
				if (capturing) {
					df_printf( "syntax error line %d\n", n_lines);
					df_printf( "\t cannot open tag over existing open tag");
					exit(1);
				} else {
					//switch to capturing, dump write buffer to disk
					capturing = true;
					i+=TAG_LENGTH_MINUS_ONE;
					buffer_write(&write_buffer, out_file);
					buffer_clear(&write_buffer);
				}

			} else if (0 == memcmp(&read_buffer[i], ENDING_TAG, 2)) {
				if (!capturing) {
					df_printf( "syntax error line %d\n", n_lines);
					df_printf( 
						"\t cannot close tag when tag has not been opened");
					exit(1);
				} else{
					// switch to writing
					capturing = false;
					i+=TAG_LENGTH_MINUS_ONE;
					// check if value in config map, else write the 
					// captured text to buffer
					d_printf("    capture \'%s\'\n",
						capture_buffer.content);
					char * relbuf = get_relation(conf, capture_buffer.content);
					if (relbuf) {
						fwrite(relbuf, sizeof(char), strlen(relbuf), out_file);
					} else {
						df_printf( 
							"cannot find entry for key \'%s\'\n",
							capture_buffer.content);
						buffer_write(&capture_buffer, out_file);
					}
					buffer_clear(&capture_buffer);

				}
			} else{
				// adding to buffer if capturing
				if(capturing) {
					// put in capture buffer
					if (!buffer_putc(&capture_buffer, read_buffer[i])) {
						df_printf( "parse error line %d\n", n_lines);
						df_printf( "\tcaptured text is greater than maximum capture_buffer size (%d)", CAPTURE_BUF_LEN);
						exit(1);
					}
				} else {
					// put in write buffer
					if (!buffer_putc(&write_buffer, read_buffer[i])) {
						df_printf( "parse error line %d\n", n_lines);
						df_printf( "\tcaptured text is greater than maximum write_buffer size (%d)", WRITE_BUF_LEN);
						exit(1);
					}
				}
			}
		}
		if (fread(read_buffer, sizeof(char), 1, raw_file)) {
			fseek(raw_file, -TAG_LENGTH, SEEK_CUR);
		}
	}

	fclose(raw_file);
	fclose(out_file);

	free(output_path);
}

static void scan_directory(xgcm_conf * conf, const char * path) {
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




char * get_output_path(xgcm_conf * conf, const char * in_path) {
    if (path_endswith(in_path, conf->file_extension)) {
        return extless_path(in_path);
    } else{
        return path_with_output_ext(conf, in_path);
    }
}

char * path_with_output_ext(xgcm_conf * conf, const char *in_path) {
    int len_in = strlen(in_path);
    int ext_len = strlen(conf->file_extension) + 6;

    char * out_path = malloc(sizeof(char) * (len_in + ext_len) );
    memcpy(out_path, in_path, len_in);
    strcat(out_path, ".");
    strcat(out_path, conf->file_extension);
    strcat(out_path, "-out");

    return out_path;
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