#include "imu.hpp"

#include <cmath>

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>

#define I2C_NODE DT_NODELABEL(i2c1)
#define IMU_I2C_ADDR 0x68
#define REG_DEVICE_CONFIG 0x11
#define REG_GYRO_CONFIG0 0x1c
#define REG_ACCEL_CONFIG0 0x1d
#define REG_WHO_AM_I 0x75
#define REG_PWR_MGMT0 0x1f
#define REG_ACCEL_DATA_X1 0x0b

static const struct device *const i2c = DEVICE_DT_GET(I2C_NODE);

static int imu_write_register(uint8_t reg, uint8_t value)
{
	uint8_t data[] = {reg, value};
	return i2c_write(i2c, data, sizeof(data), IMU_I2C_ADDR);
}

static int imu_read_registers(uint8_t reg, uint8_t *data, size_t length)
{
	return i2c_write_read(i2c, IMU_I2C_ADDR, &reg, sizeof(reg),
			      data, length);
}

int imu_init(void)
{
	if (!device_is_ready(i2c)) {
		printk("I2C controller is not ready\n");
		return -ENODEV;
	}

	int ret = imu_write_register(REG_DEVICE_CONFIG, 0x01);
	if (ret < 0) {
		printk("IMU reset failed: %d\n", ret);
		return ret;
	}

	k_msleep(10);

	uint8_t who_am_i;
	ret = imu_read_registers(REG_WHO_AM_I, &who_am_i, sizeof(who_am_i));
	if (ret < 0) {
		printk("WHO_AM_I read failed: %d\n", ret);
		return ret;
	}

	printk("WHO_AM_I: 0x%02x\n", who_am_i);
	if (who_am_i != 0x63) {
		printk("Unexpected ICM-42670-L identity\n");
		return -EINVAL;
	}

	ret = imu_write_register(REG_GYRO_CONFIG0, 0x08);
	if (ret < 0) {
		printk("Gyroscope configuration failed: %d\n", ret);
		return ret;
	}

	ret = imu_write_register(REG_ACCEL_CONFIG0, 0x08);
	if (ret < 0) {
		printk("Accelerometer configuration failed: %d\n", ret);
		return ret;
	}

	ret = imu_write_register(REG_PWR_MGMT0, 0x0f);
	if (ret < 0) {
		printk("IMU power-up failed: %d\n", ret);
		return ret;
	}

	k_msleep(50);
	printk("ICM-42670-L initialized\n");
	return 0;
}

int imu_read_sample(ImuSample &sample)
{
	uint8_t data[12];
	int ret = imu_read_registers(REG_ACCEL_DATA_X1, data, sizeof(data));
	if (ret < 0) {
		return ret;
	}

	sample.accel_x = static_cast<int16_t>((data[0] << 8) | data[1]);
	sample.accel_y = static_cast<int16_t>((data[2] << 8) | data[3]);
	sample.accel_z = static_cast<int16_t>((data[4] << 8) | data[5]);
	sample.gyro_x = static_cast<int16_t>((data[6] << 8) | data[7]);
	sample.gyro_y = static_cast<int16_t>((data[8] << 8) | data[9]);
	sample.gyro_z = static_cast<int16_t>((data[10] << 8) | data[11]);
	return 0;
}

Vec3 get_direction(ImuSample const &sample) {
	return get_direction(sample.accel_x, sample.accel_y, sample.accel_z);
}

Vec3 get_direction(int16_t const &accel_x, int16_t const &accel_y, int16_t const &accel_z) {
	float const magnitude = std::sqrt(
		static_cast<float>(accel_x) * accel_x +
		static_cast<float>(accel_y) * accel_y +
		static_cast<float>(accel_z) * accel_z);
	if (magnitude == 0.0F) {
		return Vec3();
	}
	return Vec3(accel_x / magnitude, accel_y / magnitude, accel_z / magnitude);
}

Direction get_heading(Vec3 direction) {
	if (std::abs(direction.x) > std::abs(direction.y)) {
		if (std::abs(direction.x) > std::abs(direction.z)) {
			if (direction.x > 0.0F) {
				return NORTH;
			} else {
				return SOUTH;
			}
		} else {
			if (direction.z > 0.0F) {
				return UP;
			} else {
				return DOWN;
			}
		}
	} else {
		if (std::abs(direction.y) > std::abs(direction.z)) {
			if (direction.y > 0.0F) {
				return WEST;
			} else {
				return EAST;
			}
		} else {
			if (direction.z > 0.0F) {
				return UP;
			} else {
				return DOWN;
			}
		}
	}
}
