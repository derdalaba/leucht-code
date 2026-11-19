#include "imu.hpp"
#include "led.hpp"
#include "ntc.hpp"
#include "uart.hpp"

#include <cmath>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int set_leds(Vec3 const& dir) {
	if (std::abs(dir.x) < 0.05F && std::abs(dir.y) < 0.05F) {
		if (dir.z > 0.0F) {
			return led_set_all(6, 4, 12);
		} else {
			return led_set_all(0, 0, 16);
		}
	} else {
		int const num_leds = get_num_leds();
		float angle = std::atan2(dir.y, dir.x);
		if (angle < 0.0F) {
			angle += 2.0F * 3.14159265358979323846F;
		}

		for (uint8_t i = 0; i < num_leds; i++) {
			float curr_angle = (static_cast<float>(i % num_leds) / static_cast<float>(num_leds)) * 2.0F * 3.14159265358979323846F;
			float a_y = std::sin(curr_angle);
			float a_x = std::cos(curr_angle);
			float b_y = std::sin(angle);
			float b_x = std::cos(angle);
			float brightness = (a_x * b_y) - (a_y * b_x);
			if (brightness > 0.0F) {
				led_set((i + 4) % num_leds, static_cast<int>(8 * brightness), 0, static_cast<int>(16 * brightness));
			} else {
				brightness = std::powf(std::abs(brightness), 3.0F);
				led_set((i + 4) % num_leds, static_cast<int>(16 * brightness), 0, static_cast<int>(8 * brightness));
			}
		}
		return update();
	}
}

int main(void)
{
	int32_t ntc_mv;
	int ret;

	ret = uart_init();
	if (ret < 0) {
		return 0;
	}

	ret = led_init();
	if (ret < 0) {
		return 0;
	}

	ret = led_set_all(16, 8, 0);
	if (ret < 0) {
		printk("WS2812 update failed: %d\n", ret);
		return 0;
	}

	ret = ntc_init();
	if (ret < 0) {
		return 0;
	}

	ret = imu_init();
	if (ret < 0) {
		return 0;
	}
	while (1) {
		uint8_t received;
		if (uart_read(received) == 0) {
			uart_write(&received, 1);
		}

		ImuSample sample;
		ret = imu_read_sample(sample);
		if (ret < 0) {
			printk("IMU data read failed: %d\n", ret);
			k_msleep(1000);
			continue;
		}

		ret = ntc_read_millivolts(ntc_mv);
		if (ret < 0) {
			printk("NTC read failed: %d\n", ret);
			k_msleep(1000);
			continue;
		}

		printf("accel %d %d %d, gyro %d %d %d, ntc %d mV\n",
		       sample.accel_x, sample.accel_y, sample.accel_z,
		       sample.gyro_x, sample.gyro_y, sample.gyro_z, ntc_mv);
		
		Vec3 dir = get_direction(sample);
		set_leds(dir);

		k_msleep(50);
	}
}
