#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/printk.h>

#define STRIP_NODE DT_ALIAS(led_strip)

static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);
static struct led_rgb pixel;

int main(void)
{
	printk("Leucht-Ohrringe firmware started\n");

	if (!device_is_ready(strip)) {
		printk("LED strip device is not ready\n");
		return 0;
	}

	printk("LED strip device is ready\n");

	while (1) {
		/* Set LED to Red */
		pixel.r = 0xFF; pixel.g = 0x00; pixel.b = 0x00;
		printk("LED: red\n");
		led_strip_update_rgb(strip, &pixel, 1);
		k_msleep(1000);

		/* Set LED to Green */
		pixel.r = 0x00; pixel.g = 0xFF; pixel.b = 0x00;
		printk("LED: green\n");
		led_strip_update_rgb(strip, &pixel, 1);
		k_msleep(1000);
	}
	return 0;
}