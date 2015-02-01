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

void convert_file(xgcm_conf * conf, const char * path) {
	//TODO this
	fflush(stdout);
	char * output_path = get_output_path(conf, path);
	d_printf("  processing file '%s' -> '%s'\n", 
		path, output_path);

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
		for (i=0; i<readsize; i++) {
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