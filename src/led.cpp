#include "led.hpp"

#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/sys/printk.h>

#define LED_STRIP_NODE DT_NODELABEL(ws2812)
#define LED_STRIP_COUNT 25

static const struct device *const strip =
	DEVICE_DT_GET(LED_STRIP_NODE);
static struct led_rgb pixels[LED_STRIP_COUNT];

int led_init(void)
{
	if (!device_is_ready(strip)) {
		printk("WS2812 LED strip is not ready\n");
		return -ENODEV;
	}

	return led_set_all(0, 0, 0);
}

int led_set_all(uint8_t red, uint8_t green, uint8_t blue)
{
	for (auto &pixel : pixels) {
		pixel.r = red;
		pixel.g = green;
		pixel.b = blue;
	}

	return led_strip_update_rgb(strip, pixels, LED_STRIP_COUNT);
}

int get_num_leds() {
	return LED_STRIP_COUNT;
}

int led_set(int led, uint8_t red, uint8_t green, uint8_t blue) {
	if (led < 0 || led >= LED_STRIP_COUNT) {
		return 1;
	}
	pixels[led].r = red;
	pixels[led].g = green;
	pixels[led].b = blue;
	return 0;
}

int update() {
	return led_strip_update_rgb(strip, pixels, LED_STRIP_COUNT);
}