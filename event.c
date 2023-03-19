#pragma once
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <lauxlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *target_class_name = NULL;
int target_focused = 1;
pthread_t event_thread;

/*Essa função recebe um ponteiro para um objeto Display que representa a conexão
com o servidor X11 e retorna uma string que contém o nome da classe da janela
atualmente focada. Ela usa as funções XGetInputFocus e XGetClassHint para obter
informações sobre a janela focada.*/
Window lhk_event_window_focused_get(Display *root) {
  Window focused_window;
  if (!XGetInputFocus(root, &focused_window, &(int){0})) {
    fprintf(stderr, "Erro ao obter janela focada\n");
    return None;
  }
  return focused_window;
}

char *lhk_event_window_class_get(Display *dpy) {
  Window focused_window = lhk_event_window_focused_get(dpy);
  XClassHint class_hint;
  if (focused_window == None) {
    fprintf(stderr, "Nenhuma janela focada\n");
    return NULL;
  }
  if (!XGetClassHint(dpy, focused_window, &class_hint)) {
    fprintf(stderr, "Erro ao obter dicas de classe para janela focada\n");
    return NULL;
  }

  char *class_name = strdup(class_hint.res_class);
  XFree(class_hint.res_name);
  XFree(class_hint.res_class);

  return class_name;
}

/*Esta função é responsável por atualizar a variável target_focused, que indica
se a janela atualmente focada é a janela-alvo que está sendo monitorada. Ela
chama a função lhk_event_window_class_get para obter o nome da classe da janela
atualmente focada e compara com a variável target_class_name (que é definida
globalmente). Se o nome da classe corresponder, target_focused é definido como
1, caso contrário, é definido como 0.*/
void lhk_event_target_update(Display *dpy) {
  char *class_name = lhk_event_window_class_get(dpy);
  if (class_name == NULL)
    return;
  if (target_class_name != NULL) {
    int result = !strcmp(class_name, target_class_name);
    target_focused = result;
  }
  printf("window active: %s\n", class_name);
}

/*Esta função é responsável por monitorar eventos relacionados a janelas. Ela
cria uma conexão com o servidor X11 usando a função XOpenDisplay, obtém o objeto
de janela raiz padrão usando a função DefaultRootWindow e configura a máscara de
eventos para ouvir mudanças de propriedade usando a função XSelectInput. A
função entra em um loop e usa a função XNextEvent para aguardar eventos
relacionados a janelas. Se o evento for relacionado à mudança da janela ativa, a
função lhk_event_target_update é chamada para atualizar a variável
target_focused.*/
void *lhk_event_handler_thread(void *ptr) {
  Display *display = XOpenDisplay(NULL);
  Window root_window = DefaultRootWindow(display);
  Window active_window = None;
  Atom active_window_atom = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
  XEvent event;

  XSelectInput(display, root_window, PropertyChangeMask);
  lhk_event_target_update(display);
  while (!XNextEvent(display, &event)) {
    if (event.xclient.message_type == active_window_atom) {
      Window new_window = lhk_event_window_focused_get(display);
      if(new_window != active_window) {
        active_window = new_window;
        lhk_event_target_update(display);
      }
    }
  }

  XCloseDisplay(display);
  return NULL;
}

/*Esta função é responsável por inicializar o monitoramento de eventos de
 *janelas em uma nova thread usando a função pthread_create. A função cria uma
 *nova thread que executa a função lhk_event_handler_thread.*/
void lhk_event_init() {
  pthread_create(&event_thread, NULL, &lhk_event_handler_thread, NULL);
}