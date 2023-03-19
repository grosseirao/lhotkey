#pragma once
#include "device.c"
#include "event.c"
#include <lauxlib.h>
#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <linux/input-event-codes.h>
#include <unistd.h>

/*Esta função recebe um código de evento (como o código de uma tecla ou botão do
 * mouse) e retorna o dispositivo de entrada (mouse ou teclado) que suporta esse
 * código. Ele verifica se o código é suportado pelo dispositivo de teclado e,
 * se não for, verifica se é suportado pelo dispositivo de mouse. Retorna NULL
 * se o código não é suportado por nenhum dos dispositivos.*/
struct Device *lhk_input_device_by_code(unsigned int code) {
  if (libevdev_has_event_code(keyboard_device->dev, EV_KEY, code)) {
    return keyboard_device;
  }
  if (libevdev_has_event_code(mouse_device->dev, EV_KEY, code)) {
    return mouse_device;
  }
  return NULL;
}
/*Esta função escreve um evento de entrada para um dispositivo fornecido (um
 * dispositivo de teclado ou mouse), com um código de evento e um valor
 * específico (pressionado ou solto). Se a variável global target_focused for
 * falsa, a função retorna sem fazer nada.*/
void lhk_input_code(const struct libevdev_uinput *device, unsigned int code,
                    int value) {
  if (!target_focused)
    return;
  libevdev_uinput_write_event(device, EV_KEY, code, value);
  libevdev_uinput_write_event(device, EV_SYN, SYN_REPORT, 0);
}
/*Esta função simula um "toque" de tecla, chamando lhk_input_code duas vezes
 * para pressionar e soltar a tecla. A função recebe um dispositivo e um código
 * de tecla como entrada. Se target_focused for falsa, a função não fará nada.*/
void lhk_input_key_stroke(const struct libevdev_uinput *device,
                          unsigned int code) {
  lhk_input_code(device, code, 1);
  lhk_input_code(device, code, 0);
}