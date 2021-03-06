#ifndef _VIRT_LAMP_H
#define _VIRT_LAMP_H

#include "config.h"

#define ROOM_VORTRAG 0
#define ROOM_LOUNGE 1
#define ROOM_KUECHE 2

void init_lamp_control();

void set_lamp(uint8_t room, uint8_t index, uint8_t enable);

void set_bright(uint8_t room, uint8_t index, uint8_t value);

void set_bright_all(uint8_t room, uint8_t value);

void set_lamp_all(uint8_t room, uint8_t enable);

uint8_t get_channel_brightness(uint8_t index);

uint8_t get_channel_status(void);

void set_lounge_lamp_1(uint8_t val);
void set_lounge_lamp_2(uint8_t val);

#endif
