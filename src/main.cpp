#include "imu.hpp"
#include "led.hpp"
#include "ntc.hpp"
#include "uart.hpp"
#include "fir.hpp"

#include <cmath>
#include <vector>

#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/printk.h>

std::vector<float> TAPS{
    0.000000000000000000,
    0.000000000000000000,
    -0.000139263539278841,
    0.000000000000000000,
    0.000676086428772603,
    -0.000000000000000001,
    -0.001919476171660357,
    0.000000000000000003,
    0.004375166017975936,
    -0.000000000000000005,
    -0.008783260092928008,
    0.000000000000000007,
    0.016240231768181117,
    -0.000000000000000011,
    -0.028647813504511312,
    0.000000000000000014,
    0.050452259180646081,
    -0.000000000000000017,
    -0.097653352917272021,
    0.000000000000000019,
    0.315400088254565947,
    0.499998669151017872,
    0.315400088254565947,
    0.000000000000000019,
    -0.097653352917272035,
    -0.000000000000000017,
    0.050452259180646074,
    0.000000000000000014,
    -0.028647813504511326,
    -0.000000000000000011,
    0.016240231768181127,
    0.000000000000000007,
    -0.008783260092928008,
    -0.000000000000000005,
    0.004375166017975932,
    0.000000000000000003,
    -0.001919476171660359,
    -0.000000000000000001,
    0.000676086428772605,
    0.000000000000000000,
    -0.000139263539278841,
    0.000000000000000000,
    0.000000000000000000,
};
Color PURPLE{190, 0, 255};

int set_leds(Vec3 const& dir) {
	int const num_leds = get_num_leds();
	if (std::abs(dir.x) < 0.05F && std::abs(dir.y) < 0.05F) {
		if (0 != led_set_all(0, 0, 0)) {
				printf("could not set leds\n");
				return 1;
			}
		uint8_t color_val = sys_rand8_get() % 36;
		Color col = (dir.z > 0.0F) ? Color{color_val} : Color{color_val * 0.75F, 0, color_val};
		
		for (size_t i = 0; i < 4; i++) {
			led_set(static_cast<int>(sys_rand8_get() % num_leds), col);
		}
		int ret = update();
		k_msleep(16);
		return ret;
	} else {
		
		float angle = std::atan2(dir.y, dir.x) - 0.065F; //slight error correction
		if (angle < 0.0F) {
			angle += 2.0F * 3.14159265358979323846F;
		}

		float b_y = std::sin(angle);
		float b_x = std::cos(angle);

		for (uint8_t i = 0; i < num_leds; i++) {
			float curr_angle = (static_cast<float>(i % num_leds) / static_cast<float>(num_leds)) * 2.0F * 3.14159265358979323846F;
			float a_y = std::sin(curr_angle);
			float a_x = std::cos(curr_angle);
			
			float brightness = (a_x * b_y) - (a_y * b_x);
			if (brightness > 0.0F) {
				led_set((i + 4) % num_leds, PURPLE * 0.125F * brightness);
			} else {
				brightness = std::powf(std::abs(brightness), 5.0F);
				led_set((i + 4) % num_leds, static_cast<int>(12 * brightness), 0, static_cast<int>(12 * brightness));
			}
		}
		return update();
	}
}

int main(void)
{
	Fir::Filter filter_x{TAPS};
	Fir::Filter filter_y{TAPS};
	Fir::Filter filter_z{TAPS};

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
		/*
		printf("accel %d %d %d, gyro %d %d %d, ntc %d mV\n",
		       sample.accel_x, sample.accel_y, sample.accel_z,
		       sample.gyro_x, sample.gyro_y, sample.gyro_z, ntc_mv);
		*/
		sample.accel_x = static_cast<int16_t>(
			filter_x.filter(static_cast<float>(sample.accel_x)));
		sample.accel_y = static_cast<int16_t>(
			filter_y.filter(static_cast<float>(sample.accel_y)));
		sample.accel_z = static_cast<int16_t>(
			filter_z.filter(static_cast<float>(sample.accel_z)));
		Vec3 dir = get_direction(sample);
		set_leds(dir);

		k_msleep(5);
	}
}
