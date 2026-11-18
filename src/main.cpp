#include "imu.hpp"
#include "ntc.hpp"
#include "uart.hpp"

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int main(void)
{
	int32_t ntc_mv;
	int ret;

	ret = uart_init();
	if (ret < 0) {
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

		printk("accel %d %d %d, gyro %d %d %d, ntc %d mV\n",
		       sample.accel_x, sample.accel_y, sample.accel_z,
		       sample.gyro_x, sample.gyro_y, sample.gyro_z, ntc_mv);
		k_msleep(100);
	}
}
