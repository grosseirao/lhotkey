#pragma once
#include "event.c"
#include <lauxlib.h>
#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <linux/input-event-codes.h>
#include <unistd.h>
#include "device.c"

void input_key(const struct libevdev_uinput *device, unsigned int key, int value) {
    if(!target_focused) return;
    libevdev_uinput_write_event(device, EV_KEY, key, value);
    libevdev_uinput_write_event(device, EV_SYN, SYN_REPORT, 0);
}

void input_key_stroke(const struct libevdev_uinput *device, unsigned int key) {
    input_key(device, key, 1);
    input_key(device, key, 0);
}

static int input_sleep(struct lua_State* L) {
    int milliseconds = luaL_checknumber(L, 1); 
    usleep(milliseconds * 1000);
    return 0;
}

static int input_get_key_state(struct lua_State* L) {
    int key = luaL_checknumber(L, 1);  // obtém o primeiro argumento do script
    int result = 0;
    if(libevdev_has_event_code(kbddev, EV_KEY, key)) {
        result = libevdev_get_event_value(kbddev, EV_KEY, key);
    } else {
        result = libevdev_get_event_value(mosdev, EV_KEY, key);
    }
    lua_pushboolean(L, result);
    return 1;
}

static int input_key_a(struct lua_State* L) {
    int key = luaL_checknumber(L, 1);  // obtém o primeiro argumento do script
    int value = luaL_checknumber(L, 2);  // obtém o segundo argumento do script
    if(libevdev_has_event_code(kbddev, EV_KEY, key)) {
        input_key(kbdinput, key, value);
    } else {
        input_key(mosinput, key, value);
    }
    return 0;
}

static int input_key_stroke_a(struct lua_State* L) {
    int key = luaL_checknumber(L, 1);  // obtém o primeiro argumento do script
    if(libevdev_has_event_code(kbddev, EV_KEY, key)) {
        input_key_stroke(kbdinput, key);
    } else {
        input_key_stroke(mosinput, key);
    }
    return 0;
}