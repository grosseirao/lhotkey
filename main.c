#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include "input.c"

struct libevdev* dev;
struct libevdev_uinput* uidev;
int state = 0;


char* name_target = "Kakele";
int target_focused = 0;

void* events_threatment(void* ptr);

void* hotkey(void* ptr);

int main(int argc, char const* argv[]) {
    pthread_t thread;
    pthread_create(&thread, NULL, &events_threatment, NULL);


    int fd = open("/dev/input/by-id/usb-04d9_USB_Gaming_Mouse-event-mouse", O_RDONLY);
    libevdev_new_from_fd(fd, &dev);
    libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev);
    struct input_event* ev = malloc(sizeof(struct input_event));

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

void update_target(Display* dpy) {
    Window focused;
    XClassHint *class_hint = malloc(sizeof(XClassHint));
    int revert;

    XGetInputFocus(dpy, &focused, &revert);

    printf("window id %d\n", focused);

    XGetClassHint(dpy, focused, class_hint);

    printf("class pointer: %d\n", class_hint->res_class);
    printf("Window class type: %s\n", class_hint->res_class);

    target_focused = !strcmp(class_hint->res_class, name_target);

    printf("focado no target: %d\n", target_focused);

    XFree(class_hint);
}

void* events_threatment(void* ptr) {
    Display* display = XOpenDisplay(NULL);
    Window root_window = DefaultRootWindow(display);
    XEvent event;

    XSelectInput(display, root_window, PropertyChangeMask);

    update_target(display);
    while (!XNextEvent(display, &event)) {
        if (!strcmp(XGetAtomName(display, event.xproperty.atom), "_NET_ACTIVE_WINDOW")) {
            update_target(display);
        }
    }
    XCloseDisplay(display);
}