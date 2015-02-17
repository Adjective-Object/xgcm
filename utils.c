#include <stdbool.h>
#include <wordexp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "utils.h"

int strbool(bool *b, const char* comp) {
    char * ref_true = "true";
    char * ref_false="false";
    bool matches_true, matches_false;
    int i;
    
    if (comp[0] == '1' && comp[1] == '\0') {
        *b = true;
        return 1;
    }
    if (comp[0] == '0' && comp[1] == '\0') {
        *b = true;
        return 1;
    }


    for(i=0; i<6; i++){
        if (i<5 && tolower(comp[i]) != ref_true[i]) {
            matches_true = false;
        }
        if (tolower(comp[i]) != ref_false[i]) {
            matches_false = false;
        }
    }
    
    if (matches_true) {
        *b = true;
        return 1;
    } else if (matches_false) {
        *b = false;
        return 1;
    }
    return 0;
}


bool str_endswith(const char *str, const char *suffix) {
    if (!str || !suffix)
        return 0;

    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    
    if (lensuffix >  lenstr)
        return 0;
    
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

bool path_endswith(const char *str, const char *suffix) {
    size_t lensuffix = strlen(suffix);

    char * ext = malloc ( lensuffix + 2 );
    memcpy(ext + 1, suffix, lensuffix + 1);
    *ext = '.';

    bool toret = str_endswith(str, ext);
    free(ext);
    return toret;
}

char * strip_string_whitespace(const char * str) {
    int len = strlen(str);

    int fst_nonsp = -1;
    int lst_nonsp = -1;

    int i;
    for (i=0; i<len; i++) {
        if (str[i] != ' ') {
            if (fst_nonsp == -1){
                fst_nonsp = i;
            } if (lst_nonsp < i) {
                lst_nonsp = i;
            }
        }
    }
    lst_nonsp++;

    int d = lst_nonsp - fst_nonsp;
    char * newstr = malloc(sizeof(char) * (d + 1));
    newstr[d] = '\0';
    memcpy(newstr, str+fst_nonsp, d);

    return newstr;
}


int TABS = 0;
void pdepth(FILE * stream){
    int i;
    for (i=0; i<TABS; i++){
        fprintf(stream, "  ");
    }
}

void tabup() {
    TABS++;
}

void tabdown() {
    TABS--;
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
    char * out_path = malloc(sublen + 1);
    memcpy(out_path, original_path, sublen);
    out_path[sublen] = '\0';

    return out_path;
}

char * chdir_to_parent(const char * rawpath) {
    // TODO fix this to not be string trash and instead use stdio functions
    // instead of yo mommaaa

    wordexp_t expand;
    wordexp(rawpath, &expand, 0);
    char * path = expand.we_wordv[0];

    char * parent_path = malloc(strlen(path));
    strcpy(parent_path, path);
    char * tracer = parent_path;
    char * lastslash = parent_path;

    while (*tracer != '\0') {
        if (*tracer == '/')
            lastslash = tracer;
        tracer ++;
    }

    char * shortpath = malloc(strlen(lastslash));
    strcpy(shortpath, lastslash + 1);

    if (*lastslash == '/') {
        *(lastslash + 1) = '\0';
    } else {
        fprintf(stderr, 
            "bad path '%s' supplied to function 'chdir_to_parent'\n",
            rawpath);
        exit(1);
    }

    chdir(parent_path);
    wordfree(&expand);

    return shortpath;
}
