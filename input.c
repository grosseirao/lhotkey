#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

void input_key_up(const struct libevdev_uinput *device, unsigned int key) {
    libevdev_uinput_write_event(device, EV_KEY, key, 0);
    libevdev_uinput_write_event(device, EV_SYN, SYN_REPORT, 0);
}

void input_key_down(const struct libevdev_uinput *device, unsigned int key) {
    libevdev_uinput_write_event(device, EV_KEY, key, 1);
    libevdev_uinput_write_event(device, EV_SYN, SYN_REPORT, 0);
}

void input_key_stroke(const struct libevdev_uinput *device, unsigned int key) {
    input_key_down(device, key);
    input_key_up(device, key);
}