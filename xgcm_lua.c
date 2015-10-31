#include "ini.h"
#include "utils.h"

#include <math.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "linit.h"

void rgb_from_hex_str(double rgb[3], char *str) {
    char cr[3], cg[3], cb[3];
    if (strlen(str) == 7) {
        //#XXXXXX
        cr[2] = '\0';
        cg[2] = '\0';
        cb[2] = '\0';
        
        cr[0] = str[1];
        cr[1] = str[2];
        cg[0] = str[3];
        cg[1] = str[4];
        cb[0] = str[5];
        cb[1] = str[6];
        
    } else {
        cr[1] = '\0';
        cg[1] = '\0';
        cb[1] = '\0';

        cr[0] = str[1];
        cg[0] = str[2];
        cb[0] = str[3];
    }

    int sr, sg, sb;
    sr = strtol(cr, NULL, 16);
    sg = strtol(cg, NULL, 16);
    sb = strtol(cb, NULL, 16);

    rgb[0] = sr;
    rgb[1] = sg;
    rgb[2] = sb;
}

void xyz_from_rgb(double xyz[3], double rgb[3]) {
    double vr, vg, vb;
    vr = rgb[0] / 255.0; 
    vg = rgb[1] / 255.0;
    vb = rgb[2] / 255.0;

    vr = vr > 0.04045 ? powl((vr + 0.055 ) / 1.055, 2.4) : vr / 12.92;
    vg = vg > 0.04045 ? powl((vg + 0.055 ) / 1.055, 2.4) : vg / 12.92;
    vb = vb > 0.04045 ? powl((vb + 0.055 ) / 1.055, 2.4) : vb / 12.92;

    //Observer. = 2°, Illuminant = D65
    xyz[0] = 100 * (vr * 0.4124 + vg * 0.3576 + vb * 0.1805);
    xyz[1] = 100 * (vr * 0.2126 + vg * 0.7152 + vb * 0.0722);
    xyz[2] = 100 * (vr * 0.0193 + vg * 0.1192 + vb * 0.9505);
}


void lab_from_xyz(double lab[3], double xyz[3]) {
    double vx, vy, vz;
    vx = xyz[0] / 95.047;
    vy = xyz[1] / 100.000;
    vz = xyz[2] / 108.883; 

    vx = vx > 0.008856 ? powl(vx, 0.33333333) : vx * 7.787 + 0.137931;
    vy = vy > 0.008856 ? powl(vy, 0.33333333) : vy * 7.787 + 0.137931;
    vz = vz > 0.008856 ? powl(vz, 0.33333333) : vz * 7.787 + 0.137931;

    lab[0] = ( 116 * vy ) - 16;
    lab[1] = 500 * ( vx - vy );
    lab[2] = 200 * ( vy - vz );
}

void xyz_from_lab(double xyz[3], double lab[3]) {
    double vx, vy, vz;
    vy = (lab[0] + 16) / 116;
    vx = lab[1] / 500 + vy;
    vz = vy - lab[2] / 200;

    double pvx, pvy, pvz;
    pvx = powl(vx,3);
    pvy = powl(vy,3);
    pvz = powl(vz,3);

    vx = pvx > 0.008856 ? pvx : ( vx - 0.137931 ) / 7.787;
    vy = pvy > 0.008856 ? pvy : ( vy - 0.137931 ) / 7.787;
    vz = pvz > 0.008856 ? pvz : ( vz - 0.137931 ) / 7.787;

    xyz[0] = 95.047  * vx;
    xyz[1] = 100.000 * vy;
    xyz[2] = 108.883 * vz;
}

void rgb_from_xyz(double rgb[3], double xyz[3]) {
    double vx, vy, vz;
    vx = xyz[0] / 100;
    vy = xyz[1] / 100;
    vz = xyz[2] / 100;

    double vr, vg, vb;
    vr = vx *  3.2406 + vy * -1.5372 + vz * -0.4986;
    vg = vx * -0.9689 + vy *  1.8758 + vz *  0.0415;
    vb = vx *  0.0557 + vy * -0.2040 + vz *  1.0570;

    vr = vr > 0.0031308 ? 1.055 * powl(vr, ( 1 / 2.4 )) - 0.055 : 12.92 * vr;
    vg = vg > 0.0031308 ? 1.055 * powl(vg, ( 1 / 2.4 )) - 0.055 : 12.92 * vg;
    vb = vb > 0.0031308 ? 1.055 * powl(vb, ( 1 / 2.4 )) - 0.055 : 12.92 * vb;

    rgb[0] = vr * 255;
    rgb[1] = vg * 255;
    rgb[2] = vb * 255;
}

void hex_str_from_rgb(char out_str[9], double rgb[3]) {
    out_str[0] = '#';

    sprintf((out_str + 1), "%02x", (unsigned int) rgb[0]);
    sprintf((out_str + 3), "%02x", (unsigned int) rgb[1]);
    sprintf((out_str + 5), "%02x", (unsigned int) rgb[2]);
}



void lua_eval(xgcm_configuration *conf, const char *luaCall) {
    // like luaL_dostring(conf->lua_state, luaCall);
    // but it respects lua 'return's

    luaL_dostring(conf->lua_state, luaCall);

    // if (!luaL_loadstring(conf->lua_state, luaCall))
    //    lua_pcall(conf->lua_state, 0, LUA_MULTRET, 0);
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

    // TODO check if the return value is a list, and if so, return the 
    // last element

    // .. probably also add some kind of casting from other types to string

    // if the output is a table, get the highest indexed element and return that
    if (lua_istable(conf->lua_state, -1)) {
        
        // for some reason lua_isnil was not working...
        int index = 1;
        const char * tablevalue;
        do {
            d_printf("checking table index: %d", index)
            lua_pushnumber(conf->lua_state, index);
            lua_gettable(conf->lua_state, -2);
            tablevalue = lua_tostring(conf->lua_state, -1);
            d_printf("  %s\n", tablevalue);

            index++;
            lua_pop(conf->lua_state, 1);
        } while (tablevalue != NULL);
        
        // index is now 2+ the max index of the table, so decrement it
        lua_pushnumber(conf->lua_state, index - 2);
        lua_gettable(conf->lua_state, -2);
        char * lval = lua_tostring(conf->lua_state, -1);
        d_printf("    Settled on \"%s\"\n", lval);
        char * rval = malloc(strlen(lval) + 1);
        memcpy(rval, lval, strlen(lval) + 1);
        // pop the string and the table off the stack
        lua_pop(conf->lua_state, 2);
        return rval;
    } else {
        char * rval = lua_tostring(conf->lua_state, -1);
        lua_pop(conf->lua_state, 1);
        return rval;
    }
}

int l_set_output_path(lua_State *L) {
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

int l_lab_select_lum(lua_State *L) {
    // select(dex_color_1, hex_color_2, bool darker)
    char * hexString1 = lua_tostring(L, -3);
    char * hexString2 = lua_tostring(L, -2);
    int select_lighter= lua_toboolean(L, -1);
    lua_pop(L, 3);

    double rgb1[3], xyz1[3], lab1[3],
           rgb2[3], xyz2[3], lab2[3];
    
    // convert the hex strings to their lab representation
    rgb_from_hex_str(rgb1, hexString1);
    xyz_from_rgb(xyz1, rgb1); // then to xyz
    lab_from_xyz(lab1, xyz1); // then to lab
    
    rgb_from_hex_str(rgb2, hexString2);
    xyz_from_rgb(xyz2, rgb2); // then to xyz
    lab_from_xyz(lab2, xyz2); // then to lab
    
    // if darker, return the darker of the two
    double * target = (select_lighter ^ (lab1[0] < lab2[0])) ? lab1 : lab2;

    // convert back to rgb
    xyz_from_lab(xyz1, target); // to xyz
    rgb_from_xyz(rgb1, xyz1); // to rgb
    //printf("lab: %f, %f, %f\n", target[0], target[1], target[2]);
    //printf("xyz: %f, %f, %f\n", xyz1[0], xyz1[1], xyz1[2]);
    //printf("rgb: %f, %f, %f\n", rgb1[0], rgb1[1], rgb1[2]);

    char out_str[9];
    hex_str_from_rgb(out_str, rgb1);
    //printf("output: %s\n", out_str);

    lua_pushstring(L, out_str);
    return 1;
}

int l_lab_lumdiff(lua_State *L) {
    // get the args
    char * hexString = lua_tostring(L, -2);
    lua_Number difference = lua_tonumber(L, -1);
    lua_pop(L, 2);

    double rgb[3], xyz[3], lab[3]; 
    
    // convert the hex string to an R/G/B set of doubles in [0, 255]
    rgb_from_hex_str(rgb, hexString);
    xyz_from_rgb(xyz, rgb); // then to xyz
    lab_from_xyz(lab, xyz); // then to lab

    // add the lab difference requested
    lab[0] = lab[0] + difference;
    lab[0] = lab[0] < -100 ? -100 : lab[0];
    lab[0] = lab[0] > 100  ?  100 : lab[0];

    // and convert back to rgb
    xyz_from_lab(xyz, lab); // then to lab
    rgb_from_xyz(rgb, xyz); // then to xyz

    char out_str[9];
    hex_str_from_rgb(out_str, rgb);

    lua_pushstring(L, out_str);

    return 1;
}


int l_lab_lumset(lua_State *L) {
    // get the args
    char * hexString = lua_tostring(L, -2);
    lua_Number lum = lua_tonumber(L, -1);
    lua_pop(L, 2);

    double rgb[3], xyz[3], lab[3]; 
    
    // convert the hex string to an R/G/B set of doubles in [0, 255]
    rgb_from_hex_str(rgb, hexString);
    xyz_from_rgb(xyz, rgb); // then to xyz
    lab_from_xyz(lab, xyz); // then to lab

    // add the lab difference requested
    lab[0] = lum;
    // and convert back to rgb
    xyz_from_lab(xyz, lab); // then to lab
    rgb_from_xyz(rgb, xyz); // then to xyz

    char out_str[9];
    hex_str_from_rgb(out_str, rgb);

    lua_pushstring(L, out_str);

    return 1;
}

int l_rgba_comma_repr(lua_State *L) {
    char * hexString = lua_tostring(L, -1);
    lua_pop(L, 1);
    
    double rgb[3];
    rgb_from_hex_str(rgb, hexString);

    char out_str[15];
    sprintf((out_str), "%3d", (unsigned int) rgb[0]);
    out_str[3] = ',';
    sprintf((out_str + 4), "%3d", (unsigned int) rgb[1]);
    out_str[7] = ',';
    sprintf((out_str + 8), "%3d", (unsigned int) rgb[2]);
    out_str[14] = '\0';
    
    lua_pushstring(L, out_str);

    return 1;
}

void register_xgcm_fns(lua_State *L) {
    lua_pushcfunction(L, l_set_output_path);
    lua_setglobal(L, "xgcm_output_path");

    lua_pushcfunction(L, l_lab_select_lum);
    lua_setglobal(L, "lab_select_lum");

    lua_pushcfunction(L, l_lab_lumdiff);
    lua_setglobal(L, "lab_lumdiff");

    lua_pushcfunction(L, l_lab_lumset);
    lua_setglobal(L, "lab_lumset");

    lua_pushcfunction(L, l_rgba_comma_repr);
    lua_setglobal(L, "rgba_comma_repr");
}
