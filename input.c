#pragma once
#include "event.c"
#include <lauxlib.h>
#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <linux/input-event-codes.h>
#include <unistd.h>
#include "device.c"

struct Device *lhk_input_device_by_code(unsigned int code) {
    if (libevdev_has_event_code(keyboard_device->dev, EV_KEY, code)) {
        return keyboard_device;
    } 
    if (libevdev_has_event_code(mouse_device->dev, EV_KEY, code)) {
        return mouse_device;
    }
    return NULL;
}

void lhk_input_code(const struct libevdev_uinput *device, unsigned int code, int value) {
    if(!target_focused) return;
    libevdev_uinput_write_event(device, EV_KEY, code, value);
    libevdev_uinput_write_event(device, EV_SYN, SYN_REPORT, 0);
}

void lhk_input_key_stroke(const struct libevdev_uinput *device, unsigned int code) {
    lhk_input_code(device, code, 1);
    lhk_input_code(device, code, 0);
}