#include <X11/Xlib.h>

int state = 0;
char* name_target = "Kakele";
int target_focused = 0;

void update_target(Display* dpy) {
    Window focused;
    XClassHint* class_hint = malloc(sizeof(XClassHint));
    int revert;
    if (!XGetInputFocus(dpy, &focused, &revert)) {
        printf("erro ao obter janela focada\n");
        return;
    }

    if (focused == 1) {
        printf("erro janela focada invalida\n");
        return;
    }

    if (!XGetClassHint(dpy, focused, class_hint)) {
        printf("erro ao obter classe da janela focada\n");
        return;
    }

    target_focused = !strcmp(class_hint->res_class, name_target);
    printf("window active: %s\n", class_hint->res_class);
    XFree(class_hint);
}

void* events_treatment(void* ptr) {
    Display* display = XOpenDisplay(NULL);
    Window root_window = DefaultRootWindow(display);
    XEvent* event = malloc(sizeof(XEvent));

    XSelectInput(display, root_window, PropertyChangeMask);

    update_target(display);
    while (!XNextEvent(display, event)) {
        //printf("event %s %d\n", XGetAtomName(display, event->xproperty.atom), event->xproperty.atom);
        if (strcmp(XGetAtomName(display, event->xproperty.atom),"_NET_ACTIVE_WINDOW") == 0) {
            update_target(display);
        }
    }
    
    XCloseDisplay(display);
    return NULL;
}