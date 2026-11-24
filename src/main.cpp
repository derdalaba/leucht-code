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
    -0.000000000000000002,
    0.001457826841675081,
    0.006549295164930914,
    0.016882484264206098,
    0.033750668331437424,
    0.056803882125556170,
    0.083290418346488163,
    0.108358278141449257,
    0.126410562133999788,
    0.132993169300514108,
    0.126410562133999788,
    0.108358278141449299,
    0.083290418346488204,
    0.056803882125556170,
    0.033750668331437444,
    0.016882484264206136,
    0.006549295164930921,
    0.001457826841675083,
    -0.000000000000000002,
};

Fir::Filter filter_a(TAPS);
Color PURPLE{190, 0, 255};
Color PINK{255, 0, 190};
float TWO_PI{2.0F * 3.14159265358979323846F};
float last_angle = 0.0F;
int32_t turns = 0;
bool angle_initialized = false;

int set_leds(Vec3 const& dir) {
	int const num_leds = get_num_leds();
	if (std::abs(dir.x) < 0.05F && std::abs(dir.y) < 0.05F) {
		if (0 != led_set_all(0, 0, 0)) {
				printf("could not set leds\n");
				return 1;
			}
		uint8_t color_val = sys_rand8_get() % 36;
		Color col = (dir.z > 0.0F) ? Color{color_val} : Color{static_cast<int>(static_cast<float>(color_val) * 0.75F), 0, color_val};
		
		for (size_t i = 0; i < 4; i++) {
			led_set(static_cast<int>(sys_rand8_get() % num_leds), col);
		}
		int ret = update();
		k_msleep(16);
		return ret;
	} else {
		float magnitude = std::sqrtf(std::powf(dir.x, 2.0F) + std::powf(dir.y, 2.0F));
		float angle = std::atan2(dir.y, dir.x) - 0.065F; //slight error correction
        if (angle_initialized) {
            angle += turns * TWO_PI;
            while (angle - last_angle > TWO_PI * 0.5F) {
                angle -= TWO_PI;
                turns--;
            }
            while (angle - last_angle < -TWO_PI * 0.5F) {
                angle += TWO_PI;
                turns++;
            }
        }
        last_angle = angle;
        angle_initialized = true;
        angle = angle * 0.1F;
        angle = filter_a.filter(angle);
        angle = angle * 10.0F;

		float b_y = std::sin(angle);
		float b_x = std::cos(angle);

		for (uint8_t i = 0; i < num_leds; i++) {
			float curr_angle = (static_cast<float>(i % num_leds) / static_cast<float>(num_leds)) * TWO_PI;
			float a_y = std::sin(curr_angle);
			float a_x = std::cos(curr_angle);
			
			float brightness = (a_x * b_y) - (a_y * b_x);
			if (brightness > 0.0F) {
				led_set((i + 4) % num_leds, PURPLE * 0.175F * std::powf(brightness, (1.3F - magnitude) * 2.0F));
			} else {
				brightness = std::powf(std::abs(brightness), (magnitude + 0.15F) * 6.0F);
				led_set((i + 4) % num_leds, PINK * 0.175F * brightness);
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
    int64_t start = 0;
	while (1) {
        start = k_uptime_get();
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
		sample.accel_x = static_cast<int16_t>(
			filter_x.filter(static_cast<float>(sample.accel_x)));
		sample.accel_y = static_cast<int16_t>(
			filter_y.filter(static_cast<float>(sample.accel_y)));
		sample.accel_z = static_cast<int16_t>(
			filter_z.filter(static_cast<float>(sample.accel_z)));
		*/
		Vec3 dir = get_direction(sample);
		set_leds(dir);
        printf("turns: %d\ndelta time : %dms\n", turns, static_cast<int>(k_uptime_get() - start));
		//k_msleep(5);
	}
}
