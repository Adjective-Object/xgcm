#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <wordexp.h>
#include "ini.h"
#include "utils.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

ll *TO_PARSE;
ll *WORKING_DIRS;
ll *PARSED;

xgcm_conf * CURRENT_PARSING_CONF;

void conf_init() {
    WORKING_DIRS = ll_init();
    TO_PARSE = ll_init();
    PARSED = ll_init();
}

bool relations_header = true;

int handle_ini(
        void *void_conf, const char *section,
        const char *name, const char *value) {

    xgcm_configuration *conf = (xgcm_configuration *) void_conf;

    // catch the xgcm configs
    if (MATCH("xgcm", "version")) {
        conf->version = atoi(value);
    }
    else if (MATCH("xgcm", "files")) {
        add_files(conf, value);
    }
    else if (MATCH("xgcm", "recursive")) {
        return strbool(&(conf->recursive), value);
    }
    else if (MATCH("xgcm", "follow_symlinks")) {
        return strbool(&(conf->follow_symlinks), value);
    }
    else if (MATCH("xgcm", "verbose")) {
        return strbool(&(conf->verbose), value);
    }
    else if (MATCH("xgcm", "multiline_divider")) {
        conf->multiline_divider = malloc(sizeof(char) * strlen(value) + 1);
        strcpy(conf->multiline_divider, value);
    } else if (MATCH("xgcm", "file_extension")) {
        conf->file_extension = malloc(sizeof(char) * strlen(value) + 1);
        strcpy(conf->file_extension, value);
    } else if (MATCH("xgcm", "tempdir_path")) {
        conf->tempdir_path = malloc(sizeof(char) * strlen(value) + 1);
        strcpy(conf->tempdir_path, value);
    } else if (MATCH("xgcm", "tempfile_prefix")) {
        conf->tempfile_prefix = malloc(sizeof(char) * strlen(value) + 1);
        strcpy(conf->tempfile_prefix, value);
    } else if (MATCH("xgcm", "include")) {
        enqueue_conf_file(value);
    } else if (MATCH("lua", "<block>")) {
        // interpret lua block
        df_printf("interpeting lua block:\n%s\n",value);
        if (!luaL_loadstring(conf->lua_state, value))
            lua_pcall(conf->lua_state, 0, LUA_MULTRET, 0);

        // print any resulting errors
        if(conf->verbose) {
            printf("error ");
            lua_stackDump(conf->lua_state);            
        }

        // empty the stack of all errors
        if (lua_gettop(conf->lua_state) > 0)
            lua_pop(conf->lua_state, lua_gettop(conf->lua_state));

        df_printf("---\n");
    }

    // catch the attributes
    else if (0 == strcmp("attributes", section)) {
        if (relations_header) {
            d_printf("relations:\n");
            relations_header = false;
        }
        if (strlen(value) > 0){
            d_printf("  ~ '%s': '%s'\n", name, value);
            add_relation(conf, name, value);
        }
    }

    // else print some errors
    else {
        df_printf("failed to match section='%s' name='%s' value='%s'\n",
                section, name, value);
        return 0;
    }
    return 1;
}

#define DIRLEN 4096

void enqueue_conf_file(const char *rawpath) {
    if (0 == strcmp("", rawpath)) {
        return;
    }

    wordexp_t expand;
    wordexp(rawpath, &expand, 0);
    char *path = expand.we_wordv[0];

    char *wd = malloc(DIRLEN);
    getcwd(wd, DIRLEN);

    //printf("enqueing (%s) %s\n", wd, path);

    ll_append(WORKING_DIRS, wd);
    ll_append(TO_PARSE, path);

    wordfree(&expand);
}

void parse_conf_files(xgcm_conf *conf) {
    char *oldpath = malloc(DIRLEN);
    char *fullpath = malloc(DIRLEN);

    while (TO_PARSE->head != NULL) {
        if (conf->verbose) {
            printf("\nto_parse: ");
            ll_print(TO_PARSE);
            printf("working : ");
            ll_print(WORKING_DIRS);
            printf("\n");
        }

        // change the current working directory to the parent directory of the
        // current conf file, so that wordexps from the file will work properly
        getcwd(oldpath, DIRLEN);

        char *temp_path = ll_pop_head(WORKING_DIRS);
        chdir(temp_path);

        const char *shortname = chdir_to_parent(ll_pop_head(TO_PARSE));

        getcwd(fullpath, DIRLEN);
        strncat(fullpath, "/", DIRLEN);
        strncat(fullpath, shortname, DIRLEN);

        if (!(ll_contains(PARSED, fullpath))) {
            // load the configuration file, printing errors on failure
            // printf("processing '%s'\n", fullpath);
            ll_append(PARSED, fullpath);

            if (ini_parse(shortname, handle_ini, conf) < 0) {
                fprintf(stderr,
                        "error loading the config file '%s'.\n",
                        fullpath);
                exit(1);
            }
        }
        // else{
        //     printf("already processed '%s' skipping\n", fullpath);
        // }

        // change th current working directory back to the old directory
        chdir(oldpath);
    }
    free(oldpath);
    free(fullpath);
}

int l_control_set_output_path (lua_State *L) {
    char * newpath = lua_tostring(L,1);

    // just so the macros work
    xgcm_conf * conf = CURRENT_PARSING_CONF;
    d_printf("output path set to: %s\n",newpath);
    char * expandedpath = expand_path(newpath);
    d_printf("expanded path: %s\n",expandedpath);

    CURRENT_PARSING_CONF->
        current_parse_control->
            final_path = expandedpath;

    return 0;
}

void build_default_config(xgcm_configuration *conf) {
    conf->lua_state = luaL_newstate();
    lua_pushcfunction(conf->lua_state, l_control_set_output_path);
    lua_setglobal(conf->lua_state, "xgcm_output_path");



    conf->version = 0;

    conf->recursive = true;
    conf->explore_hidden = false;
    conf->follow_symlinks = false;
    conf->verbose = false;
    conf->mutable_control = true;
    conf->current_parse_control = NULL;

    conf->files = ll_init();

    const char *deftemp = "/tmp/xgcm/";
    conf->tempdir_path = malloc(sizeof(char) * (strlen(deftemp) + 1));
    strcpy(conf->tempdir_path, deftemp);

    const char *deftemppre = "temp_";
    conf->tempfile_prefix = malloc(sizeof(char) * (strlen(deftemppre) + 1));
    strcpy(conf->tempfile_prefix, deftemppre);

    const char *defext = "xgcm";
    conf->file_extension = malloc(sizeof(char) * (strlen(defext) + 1));
    strcpy(conf->file_extension, defext);

    const char *defdivider = " ";
    conf->multiline_divider = malloc(sizeof(char) * strlen(defdivider) + 1);
    strcpy(conf->multiline_divider, defdivider);
}

void teardown_config(xgcm_configuration *conf) {
    free(conf->tempdir_path);
    free(conf->tempfile_prefix);
    free(conf->file_extension);
    free(conf->multiline_divider);
    lua_close(conf->lua_state);
}


void add_files(xgcm_configuration *conf, const char *files) {
    node *unprocessed = conf->files->head;

    ll_from_string(conf->files, files, ';');

    // traverse all the old nodes, expanding them
    if (unprocessed == NULL)
        unprocessed = conf->files->head;
    else
        unprocessed = unprocessed->next;

    while (unprocessed != NULL) {
        d_printf("expanding registered file %s ->", unprocessed->key);
        expand_file(unprocessed);
        d_printf(" %s\n", unprocessed->key);
        unprocessed = unprocessed->next;
    }

}

void expand_file(node *n) {
    wordexp_t expand;
    wordexp(n->key, &expand, 0);
    char *path = expand.we_wordv[0];
    char *rp = realpath(path, NULL);

    free(n->key);
    n->key = malloc(strlen(rp) + 1);
    strcpy(n->key, rp);

    wordfree(&expand);
    free(rp);
}

void print_files(node *head) {
    printf("[");
    while (head) {
        printf("%s, ", head->key);
        head = head->next;
    }
    printf("]\n");

}

void add_relation(
        xgcm_configuration *conf,
        const char *key, const char *value) {
    // check if the this exists
    lua_getglobal(conf->lua_state, key);
    int data_index = lua_gettop(conf->lua_state);
    int t = lua_type(conf->lua_state, -1);
    switch (t) {
        // if not initialized, push the value on the stack and assign
        case LUA_TNIL:
            d_printf("\tinserting value %s = '%s'\n", key, value);
            lua_pushstring(conf->lua_state, value);
            lua_setglobal(conf->lua_state, key);
            lua_pop(conf->lua_state, 1);
            break;

        // any individual value -> make it into a table
        case LUA_TNUMBER:
        case LUA_TSTRING:
            d_printf("\texpanding table for %s = '%s'\n", key, value);
            // add the stuff needed for a table

            lua_newtable(conf->lua_state);
            lua_pushnumber(conf->lua_state, 0);

            // shift those elements under the data element
            lua_insert (conf->lua_state, data_index);
            lua_insert (conf->lua_state, data_index);

            lua_settable(conf->lua_state, -3);

        // append to existing table
        case LUA_TTABLE:
            d_printf("\tappending to table %s value '%s'\n", key, value);
            int table_root = lua_gettop(conf->lua_state);
            int table_index = 0;

            //lua_stackDump(conf->lua_state);

            lua_pushnil(conf->lua_state);
            while (lua_next(conf->lua_state, table_root) != 0) {
                lua_pop(conf->lua_state, 1);
                table_index++;
            }
            //lua_stackDump(conf->lua_state);
           // printf("\tindex %d\n", table_index);

            //lua_stackDump(conf->lua_state);
            // insert the new value into the table at largest index
            lua_pushnumber(conf->lua_state, table_index);
            //lua_stackDump(conf->lua_state);
            lua_pushstring(conf->lua_state, value);
            //lua_stackDump(conf->lua_state);

            // lua_stackDump(conf->lua_state);

            lua_settable(conf->lua_state, -3);
            //lua_stackDump(conf->lua_state);
            //lua_setglobal(conf->lua_state, key);

            lua_setglobal(conf->lua_state, key);
            break;
    }

}


void lua_eval(xgcm_configuration *conf, const char *luaCall) {
    // like luaL_dostring(conf->lua_state, luaCall);
    // but it respects lua 'return's

    if (!luaL_loadstring(conf->lua_state, luaCall))
        lua_pcall(conf->lua_state, 0, LUA_MULTRET, 0);
}

char *lua_eval_return(xgcm_configuration *conf, const char *luaCall) {
    // like luaL_dostring(conf->lua_state, luaCall);
    // but it respects lua 'return's
    char * newLuaCall = malloc(strlen(luaCall) + 8);
    memcpy(newLuaCall,  "return ", 7);
    strcpy(newLuaCall+7, luaCall);

    if (!luaL_loadstring(conf->lua_state, newLuaCall))
        lua_pcall(conf->lua_state, 0, LUA_MULTRET, 0);

    free(newLuaCall);

    char * rval = lua_tostring(conf->lua_state, -1);
    lua_pop(conf->lua_state, 1);
    return rval;
}

char *next_path(xgcm_configuration *conf) {
    if (conf->files->head == NULL) {
        return NULL;
    }
    return ll_pop_head(conf->files);
}

char *get_relation(xgcm_configuration *conf, const char *key) {
    char *goodkey=strip_string_whitespace(key);
    lua_getglobal(conf->lua_state, goodkey);
    free(goodkey);

    char * rval = lua_tostring(conf->lua_state, -1);
    lua_pop(conf->lua_state, 1);
    return rval;
}

void print_conf(xgcm_configuration *conf, char *context) {
    printf("xgcm_conf '%s': {\n", context);
    printf("          version: %d\n", conf->version);

    printf("\n");

    printf("        recursive: %s\n", conf->recursive ? "true" : "false");
    printf("  follow_symlinks: %s\n", conf->follow_symlinks ? "true" : "false");
    printf("          verbose: %s\n", conf->verbose ? "true" : "false");
    printf("  mutable_control: %s\n", conf->mutable_control ? "true" : "false");

    printf("\n");

    printf("     tempdir_path: '%s'\n", conf->tempdir_path);
    printf("   file_extension: '%s'\n", conf->file_extension);
    printf("multiline_divider: '%s'\n", conf->multiline_divider);
    printf("}\n");
}
