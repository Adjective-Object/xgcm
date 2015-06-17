#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "xgcm_conf.h"
#include "string_buffer.h"
#include "utils.h"
#include "xgcm_parser.h"
#include "xgcm_traversal.h"

extern xgcm_conf * CURRENT_PARSING_CONF;

// helpers for stepping state at any given time

/* Begins a 'capture' section in convert_from_to */
static int begin_capture(xgcm_conf * conf, parse_state * state) {
    if (state->capturing) {
        fprintf(stderr,
                "parse error line %d\n of '%s':\n",
                state->n_lines, state->path);
        fprintf(stderr,"\t cannot open tag over existing open tag\n");
        fclose(state->raw_file);
        fclose(state->temp_file);
        return 1;
    } else {
        //switch to capturing, dump write buffer to disk
        state->capturing = true;
        csbuffer_clear(state->tag_buffer);
        buffer_write(state->write_buffer, state->temp_file);
        buffer_clear(state->write_buffer);
        // get the current char off the read buffer and put it into the cap buff
        buffer_putc(state->capture_buffer, state->read_buffer[state->i]);
    }
    return 0;
}

/* Ends a 'capture' section in convert_from_to*/
static int complete_capture(xgcm_conf * conf, parse_state * state) {
    if (!state->capturing) {
        fprintf(stderr,"parse error line %d\n of '%s':\n",
                state->n_lines, state->path);
        fprintf(stderr,
                "\t cannot close tag when tag has not been opened\n");
        fclose(state->raw_file);
        fclose(state->temp_file);
        return 1;
    }

    else {
        // switch to writing
        state->capturing = false;
        csbuffer_clear(state->tag_buffer);

       // check if value in config map, else write the
        // captured text to the capture buffer

        // if the capture begins with ~, evaluate it without return
        // (i.e. is mutating lctrl, the control struct)
        char * value_buffer;
        if (*(state->capture_buffer->content) == '~') {
            lua_eval(conf, state->capture_buffer->content + 1);
            value_buffer = "";
        } else {
            value_buffer = lua_eval_return(
                conf, state->capture_buffer->content);
        }

        if (value_buffer) {
            fwrite(value_buffer, sizeof(char), strlen(value_buffer), state->temp_file);
        } else {
            tabup();
            pdepth(stderr);
            fprintf(
                    stderr,
                    "line %d: cannot find entry for key \'%s\'\n",
                    state->n_lines,
                    state->capture_buffer->content);
            buffer_write(state->capture_buffer, state->temp_file);
            tabdown();
        }
        buffer_clear(state->capture_buffer);
        // get the current char off the read buffer and put it into the wrte buf
        buffer_putc(state->write_buffer, state->read_buffer[state->i]);

    }
    return 0;
}

/* Continues a 'capture' section in convert_from_to */ 
static int continue_collection(xgcm_conf *conf, parse_state * state) {
    if (state->capturing) {
        // error on length too long
        if (state->capture_buffer->len == CAPTURE_BUF_LEN) {
            fprintf(stderr,"parse error line %d\n of '%s':\n",
                    state->n_lines, state->path);
            fprintf(stderr,"capture exceeds maximum buffer length\n");
            fprintf(stderr,state->capture_buffer->content);
            fprintf(stderr,"\n");
            fclose(state->raw_file);
            fclose(state->temp_file);
            return 1;
        }
        // push char from tag capture buffer to the write buffer,
        // and put new char into tag capture buffer
        char popped_char = csbuffer_cycle(state->tag_buffer, 
                state->read_buffer[state->i]);
        if (popped_char && !buffer_putc(state->capture_buffer, popped_char)) {
            fprintf(stderr,"parse error line %d\n of '%s':\n",
                    state->n_lines, state->path);
            fprintf(stderr,"\tcaptured text is greater than "
                        "maximum capture_buffer size (%d)",
                        CAPTURE_BUF_LEN);
            fclose(state->raw_file);
            fclose(state->temp_file);
            return 1;
        }
    }

    else {
        // dump the write buffer if it is too large
        if (state->write_buffer->len == WRITE_BUF_LEN) {
            buffer_write(state->write_buffer, state->temp_file);
            buffer_clear(state->write_buffer);
        }
        // put item in write buffer
        char popped_char = csbuffer_cycle(state->tag_buffer, 
                state->read_buffer[state->i]);
        if (popped_char && !buffer_putc(state->write_buffer, popped_char)) {
            fprintf(stderr,"parse error line %d\n", state->n_lines);
            fprintf(stderr,"\tcaptured text is greater than maximum "
                        "write_buffer size (%d)",
                        WRITE_BUF_LEN);
            fclose(state->raw_file);
            fclose(state->temp_file);
            return 1;
        }
    }
    return 0;

}
/* Converts the contents of raw_fiile to temp_file*/
static int convert_from_to(xgcm_conf * conf, const char * path,
                            FILE *raw_file, FILE *temp_file) {

    sbuffer capture_buffer, write_buffer;
    csbuffer tag_buffer;

    char read_buffer[FBUFLEN];
    parse_state state = {
        raw_file,
        temp_file,
        &capture_buffer,
        &write_buffer,
        &tag_buffer,
        read_buffer,
        0,
        false,
        0, 0,
        path
    };

    // initialize capture buffers
    buffer_init(&capture_buffer, CAPTURE_BUF_LEN);
    buffer_init(&write_buffer, WRITE_BUF_LEN);
    csbuffer_init(&tag_buffer, TAG_LENGTH);

    int start_match = 0, end_match = 0;

    while (0 < (state.readsize = fread(read_buffer, sizeof(char),
            FBUFLEN, raw_file))) {
        for (state.i = 0; state.i < state.readsize; state.i++) {

            // step through the buffer,
            // parsing the text at the current position. Assumes either tag
            // does not begin with a newline. (A safe assumption)
            if (read_buffer[state.i] == '\n') {
                state.n_lines++;
            }

            if (start_match == TAG_LENGTH) {
                // if the capture is the starting tag, set mode to capturing
                // if the mode is already set to capturing, exit with error
                start_match = 0;
                if (begin_capture(conf, &state))
                    return 1;
            }

            else if (end_match == TAG_LENGTH) {
                // otherwise if it's the ending tag
                end_match = 0;
                if (complete_capture(conf, &state))
                    return 1;
            }

            else {
                // if the capture is neither the ending nor beginning tag,
                // dump it to the collection buffer
                continue_collection(conf, &state);
            }


            if (read_buffer[state.i] == STARTING_TAG[start_match])
                start_match ++;
            else
                start_match = 0;


            if (read_buffer[state.i] == ENDING_TAG[end_match])
                end_match ++;
            else
                end_match = 0;

        }
    }
    // dump the remainder of the buffers
    buffer_write(&write_buffer, temp_file);
    
    int ct = (tag_buffer.count < TAG_LENGTH) ? tag_buffer.count: TAG_LENGTH;
    if (ct) {
        
        // printf("insertin \"%.*s\n\"", 2, tag_buffer.contenthttp://i.imgur.com/EwU3XwU.jpg);
        // dump the tag capture buffer and the 
        // current character to the file on EOF
        for (; ct>0; ct--) {
            char c = csbuffer_cycle(&tag_buffer, '\0');
            fwrite(&c, sizeof(char), 1, temp_file);
        }
    }
    
    return 0;
}

int convert_file(xgcm_conf *conf, const char *path) {
    tabup();
    fflush(stdout);
    char *temp_path = get_temp_path(conf, path);
    d_pdepth(stdout);
    d_printf("processing file '%s' -> '%s'\n",
            path, temp_path);
    printf("  %s\n", path);

    tabup();
    // make the temporary directory for writing
    mk_temp_dir(conf);

    // open files, exiting on error
    FILE *raw_file, *temp_file;
    if ((raw_file = fopen(path, "r")) == NULL) {
        perrorf("open file '%s' for reading", path);
        return 1;
    }
    if ((temp_file = fopen(temp_path, "w+")) == NULL) {
        perrorf("open file '%s' for writing", temp_path);
        fclose(raw_file);
        return 1;
    }

    // figure out the final path and put that in the conf
    parser_control pctrl;
    pctrl.final_path = get_final_path(conf, path);
    conf->current_parse_control = &pctrl;

    // notify the lua state management that this is the current parsing config
    CURRENT_PARSING_CONF = conf;
    // change dir to the directory of the path.
    recover_working_dir();
    chdir_to_parent(path);
    // do the actual converstion
    if (convert_from_to(conf, path, raw_file, temp_file)) return 1;

    fclose(raw_file);
    tabdown();

    d_pdepth(stdout);
    d_printf("copying file from '%s' to '%s'\n",
            temp_path, pctrl.final_path);

    FILE *final_file;
    if ((final_file = fopen(pctrl.final_path, "w")) == NULL) {
        char *errmsg = malloc(200);
        sprintf(errmsg, "failed to open file '%s' for writing", pctrl.final_path);
        perror(errmsg);
        exit(1);
    }

    fseek(temp_file, 0, SEEK_SET);

    size_t read_count;
    char buf[128];
    while ((read_count = fread(&buf, sizeof(char), 128, temp_file)) > 0) {
        fwrite(&buf, sizeof(char), read_count, final_file);
    }
    fclose(final_file);
    free(pctrl.final_path);
    remove(temp_path);


    fclose(temp_file);
    free(temp_path);
    tabdown();
    return 0;
}


char *get_input_path(xgcm_conf *conf, const char *in_path) {

    //check if file exists and if it is a directory
    struct stat fstat;
    bool isDir = false;
    if (0 == lstat(in_path, &fstat) && (S_IFDIR == (fstat.st_mode & S_IFMT))) {
        isDir = true;
    }


    if (isDir || path_endswith(in_path, conf->file_extension)) {
        char *m = malloc(sizeof(char) * (strlen(in_path) + 1));
        strcpy(m, in_path);
        return m;
    } else {
        char *m = malloc(sizeof(char) *
                (strlen(in_path) + strlen(conf->file_extension) + 2));
        strcpy(m, in_path);
        strcat(m, ".");
        strcat(m, conf->file_extension);
        return m;
    }
}


char *get_temp_path(xgcm_conf *conf, const char *in_path) {
    size_t baselen = strlen(conf->tempdir_path) +
                     strlen(conf->tempfile_prefix);
    char *path = malloc(sizeof(char) * baselen + 4);
    strcpy(path, conf->tempdir_path);
    strcat(path, conf->tempfile_prefix);

    int i;
    struct stat fstat;
    for (i = 0; i < 1000; i++) {

        char numstr[4];
        sprintf(numstr, "%d", i);
        strcpy(path + baselen, numstr);

        if (0 > lstat(path, &fstat)) {
            df_printf("temp file %s\n", path);
            return path;
        }
    }
    df_printf("%s0 through %s999 already exist.\n",
            conf->tempdir_path, conf->tempdir_path);
    exit(1);
}

char *get_final_path(xgcm_conf *conf, const char *in_path) {
    if (path_endswith(in_path, conf->file_extension)) {
        return extless_path(in_path);
    } else {
        char *m = malloc(sizeof(char) * (strlen(in_path) + 1));
        strcpy(m, in_path);
        return m;
    }
}

void mk_temp_dir(xgcm_conf *conf) {
    struct stat fstat;

    // if file does not exist, make it
    if (0 > stat(conf->tempdir_path, &fstat)) {
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
