#pragma once
#include "event.c"
#include "input.c"
#include <fcntl.h>
#include <glib.h>
#include <lauxlib.h>
#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <lua.h>
#include <lualib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

lua_State *L;

struct Data {
    pthread_t thread;
    struct libevdev* dev;
    struct libevdev_uinput* uidev;
    int key;
};

void hotkey_createx() {
  lua_pushglobaltable(L);
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        if (lua_isfunction(L, -1)) {
            const char *func_name = lua_tostring(L, -2);
            if(libevdev_event_code_from_name(EV_KEY, func_name))
            printf("- %s\n", func_name);
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1); // remove a tabela global da pilha
}

struct Data* hotkey_create_data(char* device, int key) {
    struct Data* data;
    struct libevdev* dev;
    struct libevdev_uinput* uidev;
    int fd; 

    fd = open(device, O_RDONLY);
    data = malloc(sizeof(struct Data));
    libevdev_new_from_fd(fd, &dev);
    libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev);
    libevdev_grab(dev, LIBEVDEV_GRAB);

    data->dev = dev;
    data->uidev = uidev;
    data->key = key;
    
    if (
    libevdev_has_event_type(dev, EV_KEY) && 
    libevdev_has_event_code(dev, EV_KEY, BTN_LEFT) && 
    libevdev_has_event_code(dev, EV_KEY, BTN_RIGHT)) {
        mosinput = uidev;
        mosdev = dev;
    } else
    if (libevdev_has_event_type(dev, EV_KEY)) {
        kbdinput = uidev;
        kbddev = dev;
    }
    return data;
}

void hotkey_validator(void *data, void *data2) {
    lua_getglobal(L, data);
    lua_call(L, 0, 1);
}

GThreadPool* pool;
void *hotkey_device_thread(void *ptr) {
    struct Data *data = ptr;
    struct libevdev *dev = data->dev;
    struct libevdev_uinput *uidev = data->uidev;
    struct input_event ev;
    const char *key_code_name = libevdev_event_code_get_name(EV_KEY, data->key);;

    while (libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev) >= 0) {
        if(ev.code == data->key && target_focused) {
            if(ev.value == 1) g_thread_pool_push(pool, (void*)key_code_name, NULL);
            continue;
        }
        libevdev_uinput_write_event(uidev, ev.type, ev.code, ev.value);
    };
    return 0;
}

void hotkey_device_init(struct Data *data) {
    pthread_t thread;
    pthread_create(&thread, NULL, &hotkey_device_thread, data);
}

void load_script() {
    pool = g_thread_pool_new(hotkey_validator, NULL, -1, FALSE, NULL);
    struct Data *hkmouse = hotkey_create_data("/dev/input/by-id/usb-04d9_USB_Gaming_Mouse-event-mouse", BTN_RIGHT);
    struct Data *hkkbd = hotkey_create_data("/dev/input/by-id/usb-04d9_USB_Gaming_Mouse-if01-event-kbd", KEY_8);

    L = luaL_newstate();
    luaL_openlibs(L);
    // printf("%d\n", BTN_RIGHT);

    lua_pushcfunction(L, input_key_a);
    lua_setglobal(L, "input_key");

    lua_pushcfunction(L, input_sleep);
    lua_setglobal(L, "sleep");

    lua_pushcfunction(L, input_get_key_state);
    lua_setglobal(L, "get_key_state");

    lua_pushcfunction(L, input_key_stroke_a);
    lua_setglobal(L, "input_key_stroke");

    lua_pushcfunction(L, hotkey_target_set);
    lua_setglobal(L, "target");

    luaL_dofile(L, "func.lua");
    // int result = luaL_loadfile(L, "func.lua");

    // hotkey_createx();

    hotkey_device_init(hkmouse);
    hotkey_device_init(hkkbd);
}