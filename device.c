#pragma once
#include <dirent.h>
#include <fcntl.h>
#include <glib.h>
#include <lauxlib.h>
#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <lua.h>
#include <lualib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// struct que carrega um device e um input
struct Device {
  struct libevdev *dev;
  struct libevdev_uinput *input;
};

GList *devices;
struct Device *keyboard_device = NULL;
struct Device *mouse_device = NULL;

/*esta função cria um novo dispositivo a partir do caminho especificado em path.
 * Ela retorna um ponteiro para um struct Device, que contém um libevdev e um
 * libevdev_uinput. Se a criação do dispositivo falhar, a função retorna NULL.*/
struct Device *lhk_device_new(char *path) {
  struct Device *device = malloc(sizeof(struct Device));

  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    // fprintf(stderr, "Falha ao abrir o dispositivo de entrada %s\n", path);
    return NULL;
  }

  int rc = libevdev_new_from_fd(fd, &device->dev);
  if (rc < 0) {
    // fprintf(stderr, "Falha ao criar a estrutura libevdev para o dispositivo
    // de entrada %s\n", path);
    close(fd);
    return NULL;
  }

  int rui = libevdev_uinput_create_from_device(
      device->dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &device->input);
  if (rui < 0) {
    // fprintf(stderr, "Falha ao criar a estrutura libevdev_uinput para o device
    // %s\n", path);
    close(fd);
    return NULL;
  }
  // printf("created for %s\n", path);
  return device;
}

/*esta função verifica se o código especificado está presente em um dispositivo
 * de entrada. Ela verifica primeiro o dispositivo do teclado e, em seguida, o
 * dispositivo do mouse. Ela retorna um ponteiro para o struct Device
 * correspondente, ou NULL se o código não for encontrado em nenhum
 * dispositivo.*/
struct Device *lhk_device_by_code(unsigned int code) {
  if (libevdev_has_event_code(keyboard_device->dev, EV_KEY, code)) {
    return keyboard_device;
  }
  if (libevdev_has_event_code(mouse_device->dev, EV_KEY, code)) {
    return mouse_device;
  }
  return NULL;
}

/*esta função determina globalmente qual dispositivo é o teclado e qual é o
 * mouse. Ela faz isso verificando se cada dispositivo tem as teclas "A" e "8"
 * para o teclado e o botão esquerdo do mouse, respectivamente.*/
void lhk_device_global_determine() {
  GList *iter;
  for (iter = devices; iter != NULL; iter = iter->next) {
    struct Device *device = iter->data;
    if (mouse_device == NULL && libevdev_has_event_type(device->dev, EV_REL) &&
        libevdev_has_event_code(device->dev, EV_KEY, BTN_LEFT)) {
      mouse_device = device;
    } else if (keyboard_device == NULL &&
               libevdev_has_event_code(device->dev, EV_KEY, KEY_A) &&
               libevdev_has_event_code(device->dev, EV_KEY, KEY_8)) {
      keyboard_device = device;
    }
  }
}

/*esta função lê todos os dispositivos de entrada no diretório
 * /dev/input/by-id e cria um dispositivo para cada um deles usando a função
 * lhk_device_new(). Ela armazena cada dispositivo em uma lista vinculada.
 * Depois de criar todos os dispositivos, ela chama
 * lhk_device_global_determine() para determinar quais dispositivos são o
 * teclado e o mouse. A função retorna 0 se bem sucedida ou -1 se ocorrer uma
 * falha.*/
int lhk_device_init() {
  DIR *dir;
  struct dirent *entry;
  char filename[256];
  int fd, rc;

  // abre o diretório /dev/input/by-id
  dir = opendir("/dev/input/by-id");
  if (dir == NULL) {
    fprintf(stderr, "Falha ao abrir o diretório /dev/input/by-id\n");
    return -1;
  }

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_LNK ||
        DT_CHR) { // verifica se o arquivo é um dispositivo de caractere
      snprintf(filename, sizeof(filename), "/dev/input/by-id/%s",
               entry->d_name);
      // printf(const char *restrict format, ...)
      struct Device *device = lhk_device_new(filename);
      if (device != NULL) {
        devices = g_list_append(devices, device);
      }
    }
  }

  // fecha o diretório
  closedir(dir);

  lhk_device_global_determine();
  return 0;
}