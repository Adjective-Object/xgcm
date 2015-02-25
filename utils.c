#include <stdbool.h>
#include <wordexp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"

bool strbool(bool *b, const char *comp) {
    if (0 == strcmp(comp, "1") || 0 == strcmp(comp, "true")) {
        *b = true;
        return true;
    }
    if (0 == strcmp(comp, "0") || 0 == strcmp(comp, "false")) {
        *b = false;
        return true;
    }
    return false;
}


bool str_endswith(const char *str, const char *suffix) {
    if (!str || !suffix)
        return 0;

    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);

    if (lensuffix > lenstr)
        return 0;

    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

bool path_endswith(const char *str, const char *suffix) {
    size_t lensuffix = strlen(suffix);

    char *ext = malloc(lensuffix + 2);
    memcpy(ext + 1, suffix, lensuffix + 1);
    *ext = '.';

    bool toret = str_endswith(str, ext);
    free(ext);
    return toret;
}

char *strip_string_whitespace(const char *str) {
    size_t len = strlen(str);

    int fst_nonsp = -1;
    int lst_nonsp = -1;

    int i;
    for (i = 0; i < len; i++) {
        if (str[i] != ' ') {
            if (fst_nonsp == -1) {
                fst_nonsp = i;
            }
            if (lst_nonsp < i) {
                lst_nonsp = i;
            }
        }
    }
    lst_nonsp++;

    size_t d = (size_t) (lst_nonsp - fst_nonsp);
    char *newstr = malloc(sizeof(char) * (d + 1));
    newstr[d] = '\0';
    memcpy(newstr, str + fst_nonsp, d);

    return newstr;
}


int TABS = 0;

void pdepth(FILE *stream) {
    int i;
    for (i = 0; i < TABS; i++) {
        fprintf(stream, "  ");
    }
}

void tabup() {
    TABS++;
}

void tabdown() {
    TABS--;
}

char *extless_path(const char *original_path) {

    size_t len = strlen(original_path) + 1;

    // find the location of the point
    const char *last_dot = NULL;
    const char *tracer = original_path;
    while (*tracer != '\0') {
        if (*tracer == '.')
            last_dot = tracer;
        tracer++;
    }

    // if there was no period, set it to the end of the string
    if (last_dot == NULL)
        last_dot = original_path + len;

    size_t sublen = last_dot - original_path;
    char *out_path = malloc(sublen + 1);
    memcpy(out_path, original_path, sublen);
    out_path[sublen] = '\0';

    return out_path;
}

const char *chdir_to_parent(const char *rawpath) {
    // TODO fix this to not be string trash and instead use stdlib functions
    // instead of yo mommaaa

    wordexp_t expand;
    wordexp(rawpath, &expand, 0);
    char *path = expand.we_wordv[0];

    char *parent_path = malloc(strlen(path));
    strcpy(parent_path, path);
    char *tracer = parent_path;
    char *lastslash = parent_path;

    while (*tracer != '\0') {
        if (*tracer == '/')
            lastslash = tracer;
        tracer++;
    }

    char *shortpath = malloc(strlen(lastslash));
    strcpy(shortpath, lastslash + 1);

    if (*lastslash == '/') {
        *(lastslash + 1) = '\0';
    } else {
        wordfree(&expand);
        return rawpath;
    }

    chdir(parent_path);
    wordfree(&expand);

    return shortpath;
}
