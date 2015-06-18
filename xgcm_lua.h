#ifndef XGCM_LUA
#define XGCM_LUA

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h> 
#include "xgcm_conf.h"

void lua_eval(xgcm_configuration *conf, const char *luaCall);
char *lua_eval_return(xgcm_configuration *conf, const char *luaCall);

void register_xgcm_fns(lua_State *L);
int l_set_output_path (lua_State *L);

#endif
