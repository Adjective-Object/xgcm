#ifndef XGCM_TRAVERSAL
#define XGCM_TRAVERSAL
#include "xgcm_traversal.h"
#include "xgcm_parser.h"
#include "xgcm_conf.h"
#include "string_buffer.h"

#define PATH_BUF_LEN (sizeof(char) * 1000)

void convert_by_path(xgcm_conf * conf, const char * path);
void scan_directory(xgcm_conf * conf, const char * path);

char * path_with_output_ext(xgcm_conf * conf, const char * in_path);
char * extless_path(const char * in_path);

char * get_input_path(xgcm_conf * conf, const char * in_path);
char * get_output_path(xgcm_conf * conf, const char * in_path);
char * path_with_output_ext(xgcm_conf * conf, const char * in_path);
char * extless_path(const char * in_path);


#endif