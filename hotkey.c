#pragma once
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <glib.h>
#include <pthread.h>
#include <stdio.h>
#include "event.c"

struct Data {
    pthread_t thread;
    struct libevdev* dev;
    struct libevdev_uinput* uidev;
};

typedef void(*Callback)(void); 

int state = 0;
void* hotkey(void* ptr) {
    struct Data* data = ptr;
    state = 1;
    // printf("started\n");
    while (libevdev_get_event_value(data->dev, EV_KEY, BTN_RIGHT)) {
        input_key_stroke(data->uidev, BTN_LEFT);
        usleep((rand() % (129 - 95) + 95) * 1000);
    }
    // printf("ended\n");
    state = 0;
    return 0;
}

void* mouse_hotkey(void* ptr) {
    struct Data data;
    struct libevdev* dev;
    struct libevdev_uinput* uidev;
    struct input_event ev;
    int fd;

    fd = open("/dev/input/by-id/usb-04d9_USB_Gaming_Mouse-event-mouse", O_RDONLY);
    libevdev_new_from_fd(fd, &dev);
    libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev);
    libevdev_grab(dev, LIBEVDEV_GRAB);

    data.dev = dev;
    data.uidev = uidev;

    while (libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev) >= 0) {
        switch (ev.type) {
            case EV_KEY: {
                if (ev.code == BTN_RIGHT && ev.value == 1 && target_focused) {
                    if (!state) {
                        // Obtém a classe da janela

                        pthread_t thread;
                        data.thread = thread;
                        pthread_create(&thread, NULL, &hotkey, &data);
                        break;
                    }
                }
                // libevdev_uinput_write_event(uidev, EV_MSC, MSC_SCAN, libevdev_get_event_value(dev, EV_MSC, ev->code));
                libevdev_uinput_write_event(uidev, ev.type, ev.code, ev.value);
                libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
                break;
            }
            case EV_REL: {
                libevdev_uinput_write_event(uidev, ev.type, ev.code, ev.value);
                libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
                // printf("type: %s code: %d value: %d\n", libevdev_event_type_get_name(ev->type), ev->code, ev->value);
                break;
            }
        }
    };
}
int fkey_8 = 0;
void* mousekbd(void* ptr) {
    struct Data* data = ptr;
    if(fkey_8) {
        input_key_up(data->uidev, KEY_8);
        fkey_8 = 0;
    } else {
        input_key_down(data->uidev, KEY_8);
        fkey_8 = 1;
    }
    
    return 0;
}

void* mouse_kbd_hotkey(void* ptr) {
    struct Data data;
    struct libevdev* dev;
    struct libevdev_uinput* uidev;
    struct input_event ev;
    int fd;

    fd = open("/dev/input/by-id/usb-04d9_USB_Gaming_Mouse-if01-event-kbd", O_RDONLY);
    libevdev_new_from_fd(fd, &dev);
    libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev);
    libevdev_grab(dev, LIBEVDEV_GRAB);

    data.dev = dev;
    data.uidev = uidev;
    while (libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev) >= 0) {
        switch (ev.type) {
            case EV_KEY: {
                if (ev.code == KEY_8 && ev.value == 1 && target_focused) {
                    pthread_t thread;
                    data.thread = thread;
                    pthread_create(&thread, NULL, &mousekbd, &data);
                    break;
                }
                if(ev.code == KEY_8){
                    break;
                }
                // libevdev_uinput_write_event(uidev, EV_MSC, MSC_SCAN, libevdev_get_event_value(dev, EV_MSC, ev->code));
                libevdev_uinput_write_event(uidev, ev.type, ev.code, ev.value);
                libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
                break;
            }
            case EV_REL: {
                libevdev_uinput_write_event(uidev, ev.type, ev.code, ev.value);
                libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
                // printf("type: %s code: %d value: %d\n", libevdev_event_type_get_name(ev->type), ev->code, ev->value);
                break;
            }
        }
    };
}