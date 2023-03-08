#pragma once
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <fcntl.h>
#include <glib.h>

struct libevdev *kbddev;
struct libevdev_uinput *kbdinput;
struct libevdev *mosdev;
struct libevdev_uinput *mosinput;
GList *devices;

void hotkey_device_pool_init() {
    int fdk = open("/dev/input/by-id/usb-04d9_USB_Gaming_Mouse-if01-event-kbd", O_RDONLY);
    libevdev_new_from_fd(fdk, &kbddev);
    libevdev_uinput_create_from_device(kbddev, LIBEVDEV_UINPUT_OPEN_MANAGED, &kbdinput);
    libevdev_grab(kbddev, LIBEVDEV_GRAB);

    int fdm = open("/dev/input/by-id/usb-04d9_USB_Gaming_Mouse-event-mouse", O_RDONLY);
    libevdev_new_from_fd(fdm, &mosdev);
    libevdev_uinput_create_from_device(mosdev, LIBEVDEV_UINPUT_OPEN_MANAGED, &mosinput);
    libevdev_grab(mosdev, LIBEVDEV_GRAB);
}

void hotkey_submit_hotkey() {

}