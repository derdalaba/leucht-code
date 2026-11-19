cmake_minimum_required(VERSION 3.20.0)

set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/)
set(BOARD leucht-ohrringe)

set(Python3_EXECUTABLE "C:/ncs/toolchains/5f5ef0cea0/opt/bin/python.exe")

set(ZEPHYR_TOOLCHAIN_VARIANT zephyr)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(test-sdk LANGUAGES C CXX)

target_sources(app PRIVATE
	src/main.cpp
	src/uart.cpp
	src/imu.cpp
	src/ntc.cpp
	src/led.cpp
)
