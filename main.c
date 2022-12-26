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
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <string.h>
#include "input.c"

struct libevdev* dev;
struct libevdev_uinput* uidev;
int state = 0;


char* name_target = "Kakele";
int target_focused = 0;

void* events_treatment(void* ptr);
void* xcb_events_treatment(void* ptr);
void* hotkey(void* ptr);

int main(int argc, char const* argv[]) {
    pthread_t thread;
    pthread_create(&thread, NULL, &events_treatment, NULL);


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
    XClassHint* class_hint = malloc(sizeof(XClassHint));
    int revert;

    XGetInputFocus(dpy, &focused, &revert);
    if(!XGetClassHint(dpy, focused, class_hint)) return;

    target_focused = !strcmp(class_hint->res_class, name_target);

    printf("window active: %s\n", class_hint->res_class);
    
    XFree(class_hint);
}

void* events_treatment(void* ptr) {
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

void xcb_update_target(xcb_connection_t* conn) {
    xcb_window_t focused;
    xcb_get_input_focus_cookie_t focus_cookie = xcb_get_input_focus(conn);
    xcb_get_input_focus_reply_t* focus_reply = xcb_get_input_focus_reply(conn, focus_cookie, NULL);
    focused = focus_reply->focus;
    free(focus_reply);

    xcb_icccm_get_wm_class_reply_t class_hint;
    if (xcb_icccm_get_wm_class_reply(conn, xcb_icccm_get_wm_class(conn, focused), &class_hint, NULL) == 1) {
        target_focused = !strncmp(class_hint.class_name, name_target, strlen(name_target));
        xcb_icccm_get_wm_class_reply_wipe(&class_hint);
    }
    printf("class type: %s", class_hint.class_name);
}

void* xcb_events_treatment(void* ptr) {
    xcb_connection_t* conn = xcb_connect(NULL, NULL);
    xcb_screen_t* screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
    xcb_window_t root_window = screen->root;

    uint32_t values[] = { XCB_EVENT_MASK_PROPERTY_CHANGE };
    xcb_change_window_attributes(conn, root_window, XCB_CW_EVENT_MASK, values);

    xcb_update_target(conn);
    xcb_generic_event_t* event;
    while ((event = xcb_wait_for_event(conn))) {
        if (event->response_type == XCB_PROPERTY_NOTIFY) {
            xcb_property_notify_event_t* property_notify_event = (xcb_property_notify_event_t*)event;
            xcb_get_atom_name_cookie_t atom_name_cookie = xcb_get_atom_name(conn, property_notify_event->atom);
            xcb_get_atom_name_reply_t* atom_name_reply = xcb_get_atom_name_reply(conn, atom_name_cookie, NULL);
            if (strncmp(xcb_get_atom_name_name(atom_name_reply), "_NET_ACTIVE_WINDOW", strlen("_NET_ACTIVE_WINDOW")) == 0) {
                xcb_update_target(conn);
            }
            free(atom_name_reply);
        }
        free(event);
    }
    xcb_disconnect(conn);
}