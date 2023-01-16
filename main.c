#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include "input.c"
#include "device.c"
#include "event.c"

struct libevdev* dev;
struct libevdev_uinput* uidev;

void* events_treatment(void* ptr);
void* hotkey(void* ptr);

int main(int argc, char const* argv[]) {
    int fd = open("/dev/input/by-id/usb-04d9_USB_Gaming_Mouse-event-mouse", O_RDONLY);
    libevdev_new_from_fd(fd, &dev);
    libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev);
    struct input_event* ev = malloc(sizeof(struct input_event));

    pthread_t thread;
    pthread_create(&thread, NULL, &events_treatment, NULL);

    libevdev_grab(dev, LIBEVDEV_GRAB);
    while (libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, ev) >= 0) {
        switch (ev->type) {
            case EV_KEY: {
                if (ev->code == BTN_RIGHT && ev->value == 1) {
                    if (!state && target_focused) {
                        // Obtém a classe da janela
                        pthread_create(&thread, NULL, &hotkey, NULL);
                        break;
                    }
                }
                // libevdev_uinput_write_event(uidev, EV_MSC, MSC_SCAN, libevdev_get_event_value(dev, EV_MSC, ev->code));
                libevdev_uinput_write_event(uidev, ev->type, ev->code, ev->value);
                libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
                break;
            }
            case EV_REL: {
                libevdev_uinput_write_event(uidev, ev->type, ev->code, ev->value);
                libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
                // printf("type: %s code: %d value: %d\n", libevdev_event_type_get_name(ev->type), ev->code, ev->value);
                break;
            }
        }
    };
    return 0;
}

void* hotkey(void* ptr) {
    state = 1;
    // printf("started\n");
    while (libevdev_get_event_value(dev, EV_KEY, BTN_RIGHT)) {
        input_key_stroke(uidev, BTN_LEFT);
        usleep((rand() % (129 - 95) + 95) * 1000);
    }
    // printf("ended\n");
    state = 0;
    return 0;
}
