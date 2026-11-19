#pragma once

#include <cstdint>

enum Direction {
	UP,
	DOWN,
	NORTH,
	EAST,
	SOUTH,
	WEST
};

struct ImuSample {
	int16_t accel_x;
	int16_t accel_y;
	int16_t accel_z;
	int16_t gyro_x;
	int16_t gyro_y;
	int16_t gyro_z;
};

struct Vec3 {
	float x;
	float y;
	float z;

	Vec3(float x = 0.0F, float y = 0.0F, float z = 0.0F)
		: x(x), y(y), z(z) {}
};

int imu_init(void);
int imu_read_sample(ImuSample &sample);
Vec3 get_direction(ImuSample const &sample);
Vec3 get_direction(int16_t const &accel_x, int16_t const &accel_y, int16_t const &accel_z);
Direction get_heading(Vec3 direction);