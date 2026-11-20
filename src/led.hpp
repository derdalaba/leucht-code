#pragma once

#include <cstdint>
#include <type_traits>

struct Color {
	uint8_t r;
	uint8_t g;
	uint8_t b;

	Color() : r(0), g(0), b(0) {}
	Color(uint8_t brightness) : r(brightness), g(brightness), b(brightness) {}
	Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
};

namespace detail {

template<typename T>
uint8_t multiply_channel(uint8_t channel, T brightness)
{
	float const value = static_cast<float>(channel) * static_cast<float>(brightness);
	if (value <= 0.0F) {
		return 0;
	}
	if (value >= 255.0F) {
		return 255;
	}
	return static_cast<uint8_t>(value);
}

} // namespace detail

template<typename T>
typename std::enable_if<std::is_arithmetic<T>::value, Color>::type
operator*(Color const& color, T brightness)
{
	return Color(
		detail::multiply_channel(color.r, brightness),
		detail::multiply_channel(color.g, brightness),
		detail::multiply_channel(color.b, brightness));
}

template<typename T>
typename std::enable_if<std::is_arithmetic<T>::value, Color>::type
operator*(T brightness, Color const& color)
{
	return color * brightness;
}

int led_init(void);
int led_set_all(uint8_t red, uint8_t green, uint8_t blue);
int get_num_leds();
int led_set(int const& led, uint8_t const& red, uint8_t const& green, uint8_t const& blue);
int update();
int led_set(int const& led, Color const& col);
