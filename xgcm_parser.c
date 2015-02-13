#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "xgcm_traversal.h"
#include "xgcm_parser.h"
#include "xgcm_conf.h"
#include "string_buffer.h"
#include "utils.h"
#include <sys/stat.h>

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

int convert_file(xgcm_conf * conf, const char * path) {
	tabup();
	fflush(stdout);
	char * output_path = get_writing_path(conf, path);
	d_pdepth(stdout);
	d_printf("processing file '%s' -> '%s'\n", 
		path, output_path);
	printf("  %s\n", path);

	tabup();
	mk_temp_dir(conf);

	FILE *raw_file, *out_file;
	if ((raw_file = fopen(path, "r")) == NULL) {
		char * errmsg = malloc(200);
		sprintf(errmsg, "open file '%s' for reading", path);
		perror(errmsg);
		free(errmsg);
		return 1;
	}
	if ((out_file = fopen(output_path, "w+")) == NULL) {
		char * errmsg = malloc(200);
		sprintf(errmsg, "open file '%s' for writing", output_path);
		perror(errmsg);
		free(errmsg);
		fclose(raw_file);
		return 1;
	}

	bool capturing = false;

	// solve the 'split tamplate tags' problem by prepending some number of 
	// characters from the previous read to the current one
	char read_buffer[FBUFLEN];
	int i, readsize, n_lines = 1;
	
	sbuffer capture_buffer, write_buffer;
	buffer_init(&capture_buffer, CAPTURE_BUF_LEN);
	buffer_init(&write_buffer, WRITE_BUF_LEN);

	while (0 < (readsize = fread(read_buffer, 
									sizeof(char), 
									FBUFLEN - TAG_LENGTH_MINUS_ONE, 
									raw_file))) {
		for (i=0; i<readsize; i++) {
			// parse the text at the current position. Assumes either tag
			// does not begin with a newline. Generally a safe assumption
			if (read_buffer[i] == '\n') {
				n_lines++;
			}
			if (0 == memcmp(&read_buffer[i], STARTING_TAG, TAG_LENGTH)) {
				if (capturing) {
					df_printf( 
						"parse error line %d\n of '%s':\n",
						n_lines, path);
					df_printf("\t cannot open tag over existing open tag");
					fclose(raw_file);
					fclose(out_file);
					return 1;
				} else {
					//switch to capturing, dump write buffer to disk
					capturing = true;
					i+=TAG_LENGTH_MINUS_ONE;
					buffer_write(&write_buffer, out_file);
					buffer_clear(&write_buffer);
				}

			} else if (0 == memcmp(&read_buffer[i], ENDING_TAG, 2)) {
				if (!capturing) {
					df_printf("parse error line %d\n of '%s':\n",
						n_lines, path);
					df_printf(
						"\t cannot close tag when tag has not been opened");
					fclose(raw_file);
					fclose(out_file);
					return 1;

				} else{
					// switch to writing
					capturing = false;
					i+=TAG_LENGTH_MINUS_ONE;
					// check if value in config map, else write the 
					// captured text to buffer
					d_pdepth(stdout);
					d_printf("capture \'%s\'\n",
						capture_buffer.content);
					char * relbuf = get_relation(conf, capture_buffer.content);
					if (relbuf) {
						fwrite(relbuf, sizeof(char), strlen(relbuf), out_file);
					} else {
						tabup();
						pdepth(stderr);
						fprintf(
							stderr,
							"line %d: cannot find entry for key \'%s\'\n",
							n_lines,
							capture_buffer.content);
						buffer_write(&capture_buffer, out_file);
						tabdown();
					}
					buffer_clear(&capture_buffer);

				}
			} else{
				// adding to buffer if capturing
				if(capturing) {
					// 
					if(capture_buffer.len == CAPTURE_BUF_LEN){
						df_printf("parse error line %d\n of '%s':\n",
							n_lines, path);
						df_printf("capture exceedes maximum buffer length\n");
						df_printf(capture_buffer.content);
						df_printf("\n");	
						fclose(raw_file);
						fclose(out_file);
						return 1;
					}
					// put item in capture buffer
					if (!buffer_putc(&capture_buffer, read_buffer[i])) {
						df_printf("parse error line %d\n of '%s':\n",
							n_lines, path);
						df_printf( "\tcaptured text is greater than maximum capture_buffer size (%d)", CAPTURE_BUF_LEN);
						fclose(raw_file);
						fclose(out_file);
						return 1;
					}
				} else {
					// dump the write buffer if it is too large
					if(write_buffer.len == WRITE_BUF_LEN){
						buffer_write(&write_buffer, out_file);
						buffer_clear(&write_buffer);
					}
					// put item in write buffer
					if (!buffer_putc(&write_buffer, read_buffer[i])) {
						df_printf( "parse error line %d\n", n_lines);
						df_printf( "\tcaptured text is greater than maximum write_buffer size (%d)", WRITE_BUF_LEN);
						fclose(raw_file);
						fclose(out_file);
						return 1;
					}
				}
			}
		}

		if (fread(read_buffer, sizeof(char), 1, raw_file)) {
			fseek(raw_file, -TAG_LENGTH+1, SEEK_CUR);
		}
	}
	if (write_buffer.len > 0) {
		buffer_write(&write_buffer, out_file);
	}

	fclose(raw_file);
	tabdown();

	//copy from temp to output if make_temp_files, removing anything there.
	if (conf->make_temp_files) {
		char * final_path = get_output_path(conf, path);

		d_pdepth(stdout);
		d_printf("copying file from '%s' to '%s'\n",
			output_path, final_path);

		FILE *final_file;
		if ((final_file = fopen(final_path, "w")) == NULL) {
			char * errmsg = malloc(200);
			sprintf(errmsg, "failed to open file '%s' for writing", path);
			perror(errmsg);
			exit(1);
		}

		fseek(out_file, 0, SEEK_SET);

		int read_count;
		char buf[128];
		while((read_count = fread(&buf,sizeof(char), 128, out_file)) > 0){
			fwrite(&buf, sizeof(char), read_count, final_file);
		}
		fclose(final_file);
		free(final_path);
		remove(output_path);
	}


	fclose(out_file);
	free(output_path);
	tabdown();
	return 0;
}


char * get_input_path(xgcm_conf * conf, const char * in_path) {

	//check if file exists and if it is a directory
	struct stat fstat;
	bool isDir = false;
	if (0 == lstat(in_path, &fstat) && (S_IFDIR == (fstat.st_mode & S_IFMT))) {
		isDir = true;
	}


    if (isDir || path_endswith(in_path, conf->file_extension) ) {
    	char * m = malloc(sizeof(char) * (strlen(in_path) + 1));
    	strcpy(m, in_path);
        return m;
    } else {
    	char * m = malloc(sizeof(char) * 
    		(strlen(in_path) + strlen(conf->file_extension) + 2));
    	strcpy(m, in_path);
    	strcat(m, ".");
    	strcat(m, conf->file_extension);
        return m;
    }
}


char * get_writing_path(xgcm_conf * conf, const char * in_path) {
	if (conf->make_temp_files) {
		int baselen = strlen(conf->tempdir_path) + strlen(conf->tempfile_prefix);
		char * path = malloc(sizeof(char) * baselen + 4);
		strcpy(path, conf->tempdir_path);
		strcat(path, conf->tempfile_prefix);

		int i;
		struct stat fstat;
		for(i=0; i<1000; i++) {
			
			char numstr[4];
			sprintf(numstr, "%d", i);
			strcpy(path + baselen, numstr);
			
			if ( 0 > lstat(path, &fstat) ) {
				df_printf("temp file %s\n", path);
				return path;
			}
		}
		df_printf("%s0 through %s999 already exist.\n",
			conf->tempdir_path, conf->tempdir_path);
		exit(1);
	} else{
		return get_input_path(conf, in_path);
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

void mk_temp_dir(xgcm_conf * conf) {
	struct stat fstat;

	// if file does not exist, make it
	if ( 0 > stat(conf->tempdir_path, &fstat) ) {
		d_printf("directory '%s' not found, creating\n",
			conf->tempdir_path);
		if (0 > mkdir(conf->tempdir_path, 0700)) {
			perror("error creating directory\n");
		}
		return;
	}

	// if file exists, check if it is folder. else throw error and exit.
	if (S_IFDIR != (fstat.st_mode & S_IFMT)) {
		df_printf("temporary directory path '%s' exists and is not directory\n",
			conf->tempdir_path);
		exit(1);
	}
}