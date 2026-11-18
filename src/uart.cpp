#include "uart.hpp"

#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>

#define UART_NODE DT_CHOSEN(zephyr_console)

static const struct device *const uart = DEVICE_DT_GET(UART_NODE);

int uart_init(void)
{
	if (!device_is_ready(uart)) {
		printk("UART controller is not ready\n");
		return -ENODEV;
	}

	return 0;
}

int uart_write(const uint8_t *data, uint32_t length)
{
	for (uint32_t i = 0; i < length; i++) {
		uart_poll_out(uart, static_cast<char>(data[i]));
	}
	return 0;
}

int uart_read(uint8_t &byte)
{
	return uart_poll_in(uart, reinterpret_cast<unsigned char *>(&byte));
}
