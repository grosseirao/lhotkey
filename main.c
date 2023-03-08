#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <sched.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include "input.c"
#include "event.c"
#include "hotkey.c"
#include "device.c"

int main(int argc, char const* argv[]) {
    pthread_t thread;
    pthread_create(&thread, NULL, &events_treatment, NULL);
    hotkey_device_pool_init();
    load_script();
    pthread_join(thread, NULL);
    return 0;
}
