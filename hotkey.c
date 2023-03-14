#pragma once
#include "event.c"
#include "input.c"
#include "lua.c"
#include <fcntl.h>
#include <glib.h>
#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

GList *hotkeys;
GThreadPool *pool;

struct Hotkey {
  unsigned int code;
  struct Device *device;
};

void hotkey_hotkey_function(void *data, void *data2) {
  char *code_name = data;
  // printf("function lua called: %s\n", code_name);
  lhk_lua_function_call(code_name);
}

// inicializa a thread que monitora os inputs para um device
void *lhk_hotkey_thread_handler(void *data) {
  struct Hotkey *hotkey = data;
  const char *code_name = libevdev_event_code_get_name(EV_KEY, hotkey->code);
  struct input_event ev;

  while (libevdev_next_event(hotkey->device->dev, LIBEVDEV_READ_FLAG_NORMAL,
                             &ev) >= 0) {
    if (ev.code == hotkey->code && target_focused) {
      if (ev.value == 1)
        g_thread_pool_push(pool, (void *)code_name, NULL);
      continue;
    }
    libevdev_uinput_write_event(hotkey->device->input, ev.type, ev.code,
                                ev.value);
  };
  return 0;
}

void lhk_hotkey_submit_code(unsigned int code) {
  GList *iter;
  for (iter = devices; iter != NULL; iter = iter->next) {
    struct Device *device = iter->data;
    if (libevdev_has_event_code(device->dev, EV_KEY, code)) {
      const char *code_name = libevdev_event_code_get_name(EV_KEY, code);
      // printf("%s device registred\n", code_name);
      struct Hotkey *hotkey = malloc(sizeof(struct Hotkey));
      hotkey->code = code;
      hotkey->device = device;
      libevdev_grab(device->dev, LIBEVDEV_GRAB);
      pthread_t thread;
      pthread_create(&thread, NULL, &lhk_hotkey_thread_handler, hotkey);
    }
  }
}
// cria hotkey apartir das funcoes presents no script pre carregado
// void hotkey_script_hotkeys_load() {
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

void lhk_hotkey_script_hotkeys_parse() {
  GList *iter;
  for (iter = lhk_lua_function_list_get(); iter != NULL; iter = iter->next) {
    char *func_name = iter->data; // Obtém o nome da função
    int code = libevdev_event_code_from_name(EV_KEY, func_name);
    if (code >= 0) {
      lhk_hotkey_submit_code(code);
      printf("hotkey created to %s\n", func_name);
    }
  }
}

void lhk_hotkey_init() {
  pool = g_thread_pool_new(hotkey_hotkey_function, NULL, -1, FALSE, NULL);
  lhk_hotkey_script_hotkeys_parse();
}