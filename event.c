#pragma once
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <lauxlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

char* name_target = NULL;
int target_focused = 0;
pthread_t event_thread;

void lhk_event_target_update(Display* dpy) 
{
    if(!name_target) {
        target_focused = 1;
        return;
    }
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

void* lhk_event_handler_thread(void* ptr) {
    Display* display = XOpenDisplay(NULL);
    Window root_window = DefaultRootWindow(display);
    XEvent* event = malloc(sizeof(XEvent));

    XSelectInput(display, root_window, PropertyChangeMask);

    lhk_event_target_update(display);
    while (!XNextEvent(display, event)) {
        //printf("event %s %d\n", XGetAtomName(display, event->xproperty.atom), event->xproperty.atom);
        if (strcmp(XGetAtomName(display, event->xproperty.atom),"_NET_ACTIVE_WINDOW") == 0) {
            lhk_event_target_update(display);
        }
    }
    
    XCloseDisplay(display);
    return NULL;
}

void lhk_event_init() {
    pthread_create(&event_thread, NULL, &lhk_event_handler_thread, NULL);
}