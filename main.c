#include "device.c"
#include "event.c"
#include "hotkey.c"
#include "input.c"
#include "lua.c"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <fcntl.h>
#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdio.h>

int main(int argc, char const* argv[]) {
    lhk_event_init();
    lhk_device_init();
    lhk_lua_init();
    lhk_hotkey_init();
    pthread_join(event_thread, NULL);
    return 0;
}
