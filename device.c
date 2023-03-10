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


GList *devices;

struct Device {
    struct libevdev *dev;
    struct libevdev_uinput *input;
};
// cria criar um device e um input apartir do path e joga no struct Device
struct Device *hotkey_device_new(char *path) {
    struct Device *device = malloc(sizeof(struct Device));
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Falha ao abrir o dispositivo de entrada %s\n", path);
        return NULL;
    }

    int rc = libevdev_new_from_fd(fd, &device->dev);
    if (rc < 0) {
        fprintf(stderr, "Falha ao criar a estrutura libevdev para o dispositivo de entrada %s\n", path);
        close(fd);
        return NULL;
    }

    int rui = libevdev_uinput_create_from_device(device->dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &device->input);
    if (rui < 0) {
        fprintf(stderr, "Falha ao criar a estrutura libevdev_uinput para o device %s\n", path);
        close(fd);
        return NULL;
    }
    printf("created for %s\n", path);
    return device;
}

// ler todos os dispositvos em /dev/input/by-ide e cria um device e um input para cada
int hotkey_device_pool_init() {
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
            struct Device *device = hotkey_device_new(filename);
            if(device != NULL) {
                devices = g_list_append(devices, device);
            }
        }
    }

    // fecha o diretório
    closedir(dir);

    // processa os dispositivos de entrada abertos (por exemplo, exibe informações sobre cada dispositivo)
    // for (int i = 0; i < count; i++) {
    //     printf("Dispositivo de entrada %d:\n", i);
    //     printf("\tNome: %s\n", libevdev_get_name(dev[i]));
    //     printf("\tID do fornecedor: %04x\n", libevdev_get_id_vendor(dev[i]));
    //     printf("\tID do produto: %04x\n", libevdev_get_id_product(dev[i]));
    //     printf("\tVersão: %04x\n", libevdev_get_id_version(dev[i]));
    //     printf("\n");

    //     libevdev_free(dev[i]); // libera a estrutura libevdev para cada dispositivo de entrada
    // }
    return 0;
}