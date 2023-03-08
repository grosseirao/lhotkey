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
#include "hotkey.c"

int main(int argc, char const* argv[]) {
    struct input_event ev;
    pthread_t thread;
    pthread_create(&thread, NULL, &events_treatment, NULL);
    pthread_t thread2;
    pthread_create(&thread, NULL, &mouse_kbd_hotkey, NULL);
    mouse_hotkey(NULL);

    return 0;
}
