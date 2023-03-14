#pragma once
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <glib.h>
#include <unistd.h>
#include "device.c"
#include "input.c"

lua_State *L;

void lhk_lua_function_call(char *function_name) {
    lua_getglobal(L, function_name);
    lua_call(L, 0, 1);
}

// GList *lhk_lua_script_parse() {

//   lua_pushglobaltable(L); // Empilhe a tabela global no topo da pilha
//   lua_pushnil(L); // Empilhe um valor nulo na pilha (inicialmente, a chave)
//   while (lua_next(L, -2) != 0) { // Percorre a tabela global
//     if (lua_isfunction(L, -1)) { // Verifica se o valor atual é uma função
//       const char *func_name = lua_tostring(L, -2); // Obtém o nome da função
//       int code = libevdev_event_code_from_name(EV_KEY, func_name);
//       if (code >= 0) {
//         lhk_hotkey_submit_code(code);
//         printf("hotkey created to %s\n", func_name);
//       }
//     }
//     lua_pop(L, 1); // Remove o valor da função da pilha
//   }
//   lua_pop(L, 1);
// }

GList *lhk_lua_function_list_get() {
    GList *list;
    lua_pushglobaltable(L); // Empilhe a tabela global no topo da pilha
    lua_pushnil(L); // Empilhe um valor nulo na pilha (inicialmente, a chave)
    while (lua_next(L, -2) != 0) { // Percorre a tabela global
        if (lua_isfunction(L, -1)) { // Verifica se o valor atual é uma função
        const char *func_name = lua_tostring(L, -2); // Obtém o nome da função
        list = g_list_append(list, g_strdup(func_name));
        }
        lua_pop(L, 1); // Remove o valor da função da pilha
    }
    lua_pop(L, 1);
    return list;
}

static int lhk_input_sleep_lua(struct lua_State* L) {
    int milliseconds = luaL_checknumber(L, 1); 
    usleep(milliseconds * 1000);
    return 0;
}

static int lhk_input_get_key_state(struct lua_State* L) {
    int code = luaL_checknumber(L, 1);  // obtém o primeiro argumento do script
    int result = 0;
    GList *iter;
    for (iter = devices; iter != NULL; iter = iter->next) {
        struct Device *device = iter->data;
        if(libevdev_has_event_code(device->dev, EV_KEY, code)) {
            result = libevdev_get_event_value(device->dev, EV_KEY, code);
            if(result) break;
        }
    }
    // printf("device name: %s\n", libevdev_get_name(device->dev));
    lua_pushboolean(L, result);
    return 1;
}

static int lhk_input_key_lua(struct lua_State* L) {
    int code = luaL_checknumber(L, 1);  // obtém o primeiro argumento do script
    int value = luaL_checknumber(L, 2);  // obtém o segundo argumento do script
    struct Device *device = lhk_input_device_by_code(code);
    printf("device name: %s\n", libevdev_get_name(device->dev));
    lhk_input_code(device->input, code, value);
    return 0;
}

static int lhk_input_key_stroke_lua(struct lua_State* L) {
    int code = luaL_checknumber(L, 1);  // obtém o primeiro argumento do script
    struct Device *device = lhk_input_device_by_code(code);
    printf("device name: %s\n", libevdev_get_name(device->dev));
    lhk_input_key_stroke(device->input, code);
    return 0;
}

static int lhk_lua_event_target_set(struct lua_State* L) {
    char *name = strdup(luaL_checkstring(L, 1));
    name_target = name;
    target_focused = 0;
    printf("selected target: %s\n", name_target);
    return 0;
}

void lhk_lua_init() {
    L = luaL_newstate();
    luaL_openlibs(L);
    lua_pushcfunction(L, lhk_input_key_lua);
    lua_setglobal(L, "input_key");

    lua_pushcfunction(L, lhk_input_sleep_lua);
    lua_setglobal(L, "sleep");

    lua_pushcfunction(L, lhk_input_get_key_state);
    lua_setglobal(L, "get_key_state");

    lua_pushcfunction(L, lhk_input_key_stroke_lua);
    lua_setglobal(L, "input_key_stroke");

    lua_pushcfunction(L, lhk_lua_event_target_set);
    lua_setglobal(L, "target");

    luaL_dofile(L, "func.lua");
}