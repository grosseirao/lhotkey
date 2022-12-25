#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_icccm.h>
#include <pthread.h>

struct cw_t {
    pthread_t thread;
    xcb_connection_t* connection;
    xcb_window_t window;
};

void* funct(void* ptr) {
    struct cw_t *p = ptr; 
    return NULL;
}

struct cw_t* cw_new() {
    pthread_t thread;
    xcb_connection_t* connection;
    xcb_window_t window;

    struct cw_t *cw = malloc(sizeof(struct cw_t));
    cw->connection = connection;
    cw->thread = thread;
    cw->window = window;

    pthread_create(&thread, NULL, funct, NULL);
    return NULL;
}

int cw_change_wait_next(xcb_connection_t* connection, xcb_window_t window, xcb_icccm_get_wm_class_reply_t* wm_class) {
    if (xcb_icccm_get_wm_class_reply(connection, xcb_icccm_get_wm_class(connection, window), wm_class, NULL)) {
        printf("Classe da janela ativa: %s\n", wm_class->class_name);
        // xcb_icccm_get_wm_class_reply_wipe(&wm_class);
        return 1;
    }
    else {
        fprintf(stderr, "Falha ao obter a classe da janela\n");
        return -1;
    }
}

