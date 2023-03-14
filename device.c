#pragma once
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <fcntl.h>
#include <glib.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

// struct que carrega um device e um input
struct Device {
    struct libevdev *dev;
    struct libevdev_uinput *input;
};

GList *devices;
struct Device *keyboard_device = NULL;
struct Device *mouse_device = NULL;

// cria criar um device e um input apartir do path e joga no struct Device
struct Device *lhk_device_new(char *path) {
    struct Device *device = malloc(sizeof(struct Device));
    
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        // fprintf(stderr, "Falha ao abrir o dispositivo de entrada %s\n", path);
        return NULL;
    }

    int rc = libevdev_new_from_fd(fd, &device->dev);
    if (rc < 0) {
        // fprintf(stderr, "Falha ao criar a estrutura libevdev para o dispositivo de entrada %s\n", path);
        close(fd);
        return NULL;
    }

    int rui = libevdev_uinput_create_from_device(device->dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &device->input);
    if (rui < 0) {
        // fprintf(stderr, "Falha ao criar a estrutura libevdev_uinput para o device %s\n", path);
        close(fd);
        return NULL;
    }
    // printf("created for %s\n", path);
    return device;
}

struct Device *lhk_device_by_code(unsigned int code) {
    if (libevdev_has_event_code(keyboard_device->dev, EV_KEY, code)) {
        return keyboard_device;
    } 
    if (libevdev_has_event_code(mouse_device->dev, EV_KEY, code)) {
        return mouse_device;
    }
    return NULL;
}

void lhk_device_global_determine() {
    GList *iter;
    for (iter = devices; iter != NULL; iter = iter->next) {
        struct Device *device = iter->data;
        if (mouse_device == NULL && 
            libevdev_has_event_type(device->dev, EV_REL) && 
            libevdev_has_event_code(device->dev, EV_KEY, BTN_LEFT)) {
            mouse_device = device;
        } else
        if (keyboard_device == NULL && 
            libevdev_has_event_code(device->dev, EV_KEY, KEY_A) &&
            libevdev_has_event_code(device->dev, EV_KEY, KEY_8)) {
            keyboard_device = device;
        }
    }
}

// ler todos os dispositvos em /dev/input/by-ide e cria um device e um input para cada
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

    // lê cada arquivo no diretório e abre cada um deles como um dispositivo de entrada
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_LNK || DT_CHR) { // verifica se o arquivo é um dispositivo de caractere
            snprintf(filename, sizeof(filename), "/dev/input/by-id/%s", entry->d_name);
            // printf(const char *restrict format, ...)
            struct Device *device = lhk_device_new(filename);
            if(device != NULL) {
                devices = g_list_append(devices, device);
            }
        }
    }

    // fecha o diretório
    closedir(dir);

    lhk_device_global_determine();
    return 0;
}