#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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


void buffer_init(sbuffer * b, int maxlen) {
    b->content = malloc(sizeof(char) * (maxlen + 1));
    b->content[0] = '\0';
    b->maxlen = maxlen;
    b->len = 0;
}

bool buffer_putc(sbuffer * b, char c){
    if (b->len >= b->maxlen)
        return false;
    b->content[b->len] = c;
    b->len++;
    b->content[b->len] = '\0';
    return true;
}

void buffer_write(sbuffer * b, FILE * f){  
    fwrite(b->content,
            sizeof(char),
            b->len,
            f);
}

void buffer_clear(sbuffer * b){
    b->content[0] = '\0';
    b->len = 0;
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

    printf("%d %d\n",fst_nonsp, lst_nonsp);

    int d = lst_nonsp - fst_nonsp;
    char * newstr = malloc(sizeof(char) * (d + 1));
    newstr[d] = '\0';
    memcpy(newstr, str+fst_nonsp, d);

    return newstr;
}
