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

/*uma função que é chamada quando uma hotkey é acionada. Ela chama uma função
do Lua com o nome do código da hotkey como argumento.*/
void hotkey_hotkey_function(void *data, void *data2) {
  char *code_name = data;
  // printf("function lua called: %s\n", code_name);
  lhk_lua_function_call(code_name);
}

/*uma função que é executada em uma thread separada para monitorar os eventos de
entrada para um dispositivo específico e verificar se uma hotkey foi acionada.*/
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

/*uma função que itera através de uma lista de dispositivos de entrada e
verifica se um código de evento específico está presente. Se estiver presente,
cria uma hotkey para o código de evento especificado.*/
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

/*uma função que itera através de uma lista de funções definidas em um script do
Lua e cria hotkeys para aquelas que correspondem a um código de evento válido.*/
void lhk_hotkey_script_hotkeys_parse() {
  GList *iter;
  for (iter = lhk_lua_function_list_get(); iter != NULL; iter = iter->next) {
    char *func_name = iter->data; // Obtém o nome da função
    // lista as funçoes presentes no escopo global do luastate apos carregar as
    // funçoes do script printf("script contains function: %s\n", func_name);
    int code = libevdev_event_code_from_name(EV_KEY, func_name);
    if (code >= 0) {
      lhk_hotkey_submit_code(code);
      printf("hotkey created: %s\n", func_name);
    }
  }
}

/*uma função que inicializa o programa criando uma pool de threads e lendo as
 * hotkeys definidas em um script do Lua.*/
void lhk_hotkey_init() {
  pool = g_thread_pool_new(hotkey_hotkey_function, NULL, -1, FALSE, NULL);
  lhk_hotkey_script_hotkeys_parse();
}