#include <zephyr/kernel.h>
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

int main(void)
{
	uint8_t who_am_i;
	uint8_t data[12];
	int16_t accel_x;
	int16_t accel_y;
	int16_t accel_z;
	int16_t gyro_x;
	int16_t gyro_y;
	int16_t gyro_z;
	int ret;

	printk("ICM-42670-L raw I2C test started\n");

	if (!device_is_ready(i2c)) {
		printk("I2C controller is not ready\n");
		return 0;
	}

	ret = imu_write_register(REG_DEVICE_CONFIG, 0x01);
	if (ret < 0) {
		printk("IMU reset failed: %d\n", ret);
		return 0;
	}

	k_msleep(10);

	ret = imu_read_registers(REG_WHO_AM_I, &who_am_i, sizeof(who_am_i));
	if (ret < 0) {
		printk("WHO_AM_I read failed: %d\n", ret);
		return 0;
	}

	printk("WHO_AM_I: 0x%02x\n", who_am_i);
	if (who_am_i != 0x63) {
		printk("Unexpected ICM-42670-L identity\n");
		return 0;
	}

	ret = imu_write_register(REG_GYRO_CONFIG0, 0x08);
	if (ret < 0) {
		printk("Gyroscope configuration failed: %d\n", ret);
		return 0;
	}

	ret = imu_write_register(REG_ACCEL_CONFIG0, 0x08);
	if (ret < 0) {
		printk("Accelerometer configuration failed: %d\n", ret);
		return 0;
	}

	ret = imu_write_register(REG_PWR_MGMT0, 0x0f);
	if (ret < 0) {
		printk("IMU power-up failed: %d\n", ret);
		return 0;
	}

	k_msleep(50);
	printk("IMU powered up\n");

	while (1) {
		ret = imu_read_registers(REG_ACCEL_DATA_X1, data, sizeof(data));
		if (ret < 0) {
			printk("IMU data read failed: %d\n", ret);
			k_msleep(1000);
			continue;
		}

		accel_x = (int16_t)((data[0] << 8) | data[1]);
		accel_y = (int16_t)((data[2] << 8) | data[3]);
		accel_z = (int16_t)((data[4] << 8) | data[5]);
		gyro_x = (int16_t)((data[6] << 8) | data[7]);
		gyro_y = (int16_t)((data[8] << 8) | data[9]);
		gyro_z = (int16_t)((data[10] << 8) | data[11]);

		printk("accel %d %d %d, gyro %d %d %d\n",
		       accel_x, accel_y, accel_z,
		       gyro_x, gyro_y, gyro_z);
		k_msleep(100);
	}
}
