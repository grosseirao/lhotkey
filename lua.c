#pragma once
#include "device.c"
#include "input.c"
#include "libevdev/libevdev.h"
#include <bits/pthreadtypes.h>
#include <glib.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

lua_State *L;

/*Esta função recebe um nome de função Lua como parâmetro e chama a função
correspondente. A função Lua é obtida a partir do objeto lua_State, que deve ser
criado anteriormente e inicializado com as funções que serão expostas. O
resultado da função é empilhado na pilha do objeto lua_State.*/
void lhk_lua_function_call(char *function_name) {
  lua_getglobal(L, function_name);
  lua_call(L, 0, 1);
}

/*Esta função percorre a tabela global do objeto lua_State em busca de funções
Lua e cria uma lista encadeada de strings com os nomes das funções encontradas.
A função retorna um ponteiro para o início da lista.*/
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

/*Esta função é uma função Lua que recebe um valor numérico como argumento e
 * pausa a execução por esse número de milissegundos. A função retorna 0.*/
static int lhk_input_sleep_lua(struct lua_State *L) {
  int milliseconds = luaL_checknumber(L, 1);
  usleep(milliseconds * 1000);
  return 0;
}

/*Esta função é uma função Lua que recebe um valor numérico como argumento e
verifica se a tecla correspondente está pressionada ou não. A função percorre
uma lista encadeada de dispositivos de entrada em busca do código de evento da
tecla e retorna 1 se a tecla estiver pressionada e 0 caso contrário.*/
static int lhk_input_get_key_state(struct lua_State *L) {
  int code = luaL_checknumber(L, 1); // obtém o primeiro argumento do script
  int result = 0;
  GList *iter;
  for (iter = devices; iter != NULL; iter = iter->next) {
    struct Device *device = iter->data;
    if (libevdev_has_event_code(device->dev, EV_KEY, code)) {
      result = libevdev_get_event_value(device->dev, EV_KEY, code);
      if (result)
        break;
    }
  }
  // printf("device name: %s\n", libevdev_get_name(device->dev));
  lua_pushboolean(L, result);
  return 1;
}

/*Esta função é uma função Lua que recebe dois argumentos: um código de evento e
um valor numérico. A função simula a pressão da tecla correspondente no
dispositivo de entrada correspondente. A função retorna 0.*/
static int lhk_input_key_lua(struct lua_State *L) {
  int code;
  if (lua_isnumber(L, 1)) {
    // o primeiro argumento é um número
    code = luaL_checknumber(L, 1);
  } else {
    // o primeiro argumento é uma string
    code = libevdev_event_code_from_code_name(lua_tostring(L, 1));
  }
  int value = luaL_checknumber(L, 2); // obtém o segundo argumento do script
  struct Device *device = lhk_input_device_by_code(code);
  // printf("device name: %s\n", libevdev_get_name(device->dev));
  lhk_input_code(device->input, code, value);
  return 0;
}

/*Esta função é uma função Lua que recebe um código de evento como argumento e
simula o pressionamento e o soltamento da tecla correspondente. A função retorna
0.*/
static int lhk_input_key_stroke_lua(struct lua_State *L) {
  int code = luaL_checknumber(L, 1); // obtém o primeiro argumento do script
  struct Device *device = lhk_input_device_by_code(code);
  // printf("device name: %s\n", libevdev_get_name(device->dev));
  lhk_input_key_stroke(device->input, code);
  return 0;
}

/*Esta função é uma função Lua que recebe uma string como argumento e define uma
variável global com o nome da classe de janela alvo para eventos. A função
retorna 0.*/
static int lhk_lua_event_target_set(struct lua_State *L) {
  char *name = strdup(luaL_checkstring(L, 1));
  target_class_name = name;
  target_focused = 0;
  printf("selected target: %s\n", target_class_name);
  return 0;
}

/* Esta função é responsável por inicializar o objeto lua_State e expor as
funções Lua necessárias para o programa. A função recebe o nome do arquivo Lua
que será executado e o executa com a função luaL_dofile(). As funções Lua
necessárias para o programa são definidas com lua_pushcfunction() e adicionadas
à tabela global com lua_setglobal().*/
void lhk_lua_init(char *filename) {
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

  if (filename != NULL) {
    luaL_dofile(L, filename);
  } else {
    luaL_dofile(L, "kakele.lua");
  }
}