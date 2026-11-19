#pragma once

#include <cstdint>

int led_init(void);
int led_set_all(uint8_t red, uint8_t green, uint8_t blue);
int get_num_leds();
int led_set(int led, uint8_t red, uint8_t green, uint8_t blue);
int update();
