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
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_icccm.h>
#include "input.c"
#include "cwin.c"

struct libevdev* dev;
struct libevdev_uinput* uidev;
int state = 0;


char* name_target = "Kakele";
int target_focused;

void* xcb_events_callback(void* ptr);

void* hotkey(void* ptr);

int main(int argc, char const* argv[]) {
    pthread_t thread;
    pthread_create(&thread, NULL, &xcb_events_callback, NULL);


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

    // pthread_t evthread;
    // pthread_create(&evthread, NULL, &xevents, NULL);


    // struct input_event* ev = malloc(sizeof(struct input_event));
    // pthread_t thread;

    // int fd = open("/dev/input/by-id/usb-04d9_USB_Gaming_Mouse-event-mouse", O_RDONLY);
    // libevdev_new_from_fd(fd, &dev);
    // libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev);

    // //time_t now;
    // struct timeval tv;
    // char timestamp[32];

    // //libevdev_grab(dev, LIBEVDEV_GRAB);
    // while (libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, ev) >= 0) {
    //     // gettimeofday(&tv, NULL);
    //     // strftime(timestamp, sizeof(timestamp), "[%H:%M:%S", localtime(&tv.tv_sec));
    //     // printf("%s:%03ld] type: %s code: %s value: %d\n",
    //     //     timestamp, tv.tv_usec / 1000,
    //     //     libevdev_event_type_get_name(ev->type),
    //     //     libevdev_event_code_get_name(ev->type, ev->code),
    //     //     ev->value);
    //     switch (ev->type) {
    //     case EV_KEY: {
    //         if (ev->code == BTN_RIGHT && ev->value == 1) {
    //             if (!state) {
    //                 // Display* display;
    //                 // Window window;
    //                 // XClassHint class_hint;
    //                 // int status;

    //                 // display = XOpenDisplay(NULL);
    //                 // int revert;
    //                 // XGetInputFocus(display, &window, &revert);

    //                 // status = XGetClassHint(display, window, &class_hint);
    //                 // if (status == 0) {
    //                 //     fprintf(stderr, "Failed to get class hint\n");
    //                 // }

    //                 // printf("Window class name: %s\n", class_hint.res_name);
    //                 // printf("Window class type: %s\n", class_hint.res_class);


    //                 // XFree(class_hint.res_name);
    //                 // XFree(class_hint.res_class);

    //                 // XCloseDisplay(display);
    //                 pthread_create(&thread, NULL, &hotkey, NULL);
    //             }
    //             break;
    //         }
    //         // libevdev_uinput_write_event(uidev, EV_MSC, MSC_SCAN, libevdev_get_event_value(dev, EV_MSC, ev->code));
    //         libevdev_uinput_write_event(uidev, ev->type, ev->code, ev->value);
    //         libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    //         break;
    //     }
    //     case EV_REL: {
    //         libevdev_uinput_write_event(uidev, ev->type, ev->code, ev->value);
    //         libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    //         // printf("type: %s code: %d value: %d\n", libevdev_event_type_get_name(ev->type), ev->code, ev->value);
    //         break;
    //     }
    //     };
    // }

    // libevdev_grab(dev, LIBEVDEV_UNGRAB);
    // libevdev_free(dev);    // pthread_t thread;XClassHint class_hint;

    // pthread_create(&thread, NULL, &xcb_events_callback, NULL);
    // pthread_join(thread, NULL);
    // printf("finished\n");
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
    XClassHint class_hint;
    int revert;
    XGetInputFocus(dpy, &focused, &revert);

    XGetClassHint(dpy, focused, &class_hint);

    target_focused = !strcmp(class_hint.res_class, name_target);

    // printf("Window class type: %s\n", class_hint.res_class);
    // printf("focado no target: %d\n", target_focused);

    XFree(class_hint.res_name);
    XFree(class_hint.res_class);
}

void* xcb_events_callback(void* ptr) {
    Display* display = XOpenDisplay(NULL);
    Window root_window = DefaultRootWindow(display);
    XEvent event;

    XSelectInput(display, root_window, PropertyChangeMask);

    update_target(display);
    while (1) {
        XNextEvent(display, &event);
        if (!strcmp(XGetAtomName(display, event.xproperty.atom), "_NET_ACTIVE_WINDOW")) {
            update_target(display);
        }
    }
    XCloseDisplay(display);
}