#pragma once

#include <cstdint>

int uart_init(void);
int uart_write(const uint8_t *data, uint32_t length);
int uart_read(uint8_t &byte);
