#pragma once

#include <cstdint>

struct ImuSample {
	int16_t accel_x;
	int16_t accel_y;
	int16_t accel_z;
	int16_t gyro_x;
	int16_t gyro_y;
	int16_t gyro_z;
};

int imu_init(void);
int imu_read_sample(ImuSample &sample);
