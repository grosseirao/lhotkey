#pragma once
#include "event.c"
#include <lauxlib.h>
#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <linux/input-event-codes.h>
#include <unistd.h>
#include "device.c"

struct Device *keyboard_device = NULL;
struct Device *mouse_device = NULL;

struct Device *input_device_by_code(unsigned int code) {
    if (libevdev_has_event_code(keyboard_device->dev, EV_KEY, code)) {
        return keyboard_device;
    } 
    if (libevdev_has_event_code(mouse_device->dev, EV_KEY, code)) {
        return mouse_device;
    }
    return NULL;
}

void input_find_devices() {
    GList *iter;
    for (iter = devices; iter != NULL; iter = iter->next) {
        struct Device *device = iter->data;
        if (mouse_device == NULL && libevdev_has_event_type(device->dev, EV_REL) && libevdev_has_event_type(device->dev, EV_KEY)) {
            mouse_device = device;
        } else
        if (keyboard_device == NULL && libevdev_has_event_type(device->dev, EV_KEY) && !libevdev_has_event_type(device->dev, EV_REL)) {
            keyboard_device = device;
        }
    }
}

void input_code(const struct libevdev_uinput *device, unsigned int code, int value) {
    if(!target_focused) return;
    libevdev_uinput_write_event(device, EV_KEY, code, value);
    libevdev_uinput_write_event(device, EV_SYN, SYN_REPORT, 0);
}

void input_key_stroke(const struct libevdev_uinput *device, unsigned int code) {
    input_code(device, code, 1);
    input_code(device, code, 0);
}

static int input_sleep(struct lua_State* L) {
    int milliseconds = luaL_checknumber(L, 1); 
    usleep(milliseconds * 1000);
    return 0;
}

static int input_get_key_state(struct lua_State* L) {
    int code = luaL_checknumber(L, 1);  // obtém o primeiro argumento do script
    struct Device *device = input_device_by_code(code);
    int result = libevdev_get_event_value(device->dev, EV_KEY, code);
    lua_pushboolean(L, result);
    return 1;
}

static int input_key_a(struct lua_State* L) {
    int code = luaL_checknumber(L, 1);  // obtém o primeiro argumento do script
    int value = luaL_checknumber(L, 2);  // obtém o segundo argumento do script
    struct Device *device = input_device_by_code(code);
    int result = libevdev_get_event_value(device->dev, EV_KEY, code);
    printf("device name: %s\n", libevdev_get_name(device->dev));
    input_code(mouse_device->input, code, value);
    return 0;
}

static int input_key_stroke_a(struct lua_State* L) {
    int code = luaL_checknumber(L, 1);  // obtém o primeiro argumento do script
    struct Device *device = input_device_by_code(code);
    printf("device name: %s\n", libevdev_get_name(device->dev));
    input_key_stroke(device->input, code);
    return 0;
}