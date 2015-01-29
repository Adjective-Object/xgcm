#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

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
