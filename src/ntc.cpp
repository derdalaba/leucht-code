#include "ntc.hpp"

#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/sys/printk.h>

#define NTC_ADC_NODE DT_PATH(zephyr_user)

static const struct adc_dt_spec ntc_adc =
	ADC_DT_SPEC_GET(NTC_ADC_NODE);

int ntc_init(void)
{
	int ret;

	if (!device_is_ready(ntc_adc.dev)) {
		ret = device_init(ntc_adc.dev);
		if (ret < 0) {
			printk("ADC initialization failed: %d\n", ret);
			return ret;
		}
	}

	if (!device_is_ready(ntc_adc.dev)) {
		printk("ADC is not ready\n");
		return -ENODEV;
	}

	ret = adc_channel_setup_dt(&ntc_adc);
	if (ret < 0) {
		printk("ADC channel setup failed: %d\n", ret);
	}
	return ret;
}

int ntc_read_millivolts(int32_t &millivolts)
{
	int16_t raw;
	struct adc_sequence sequence = {};

	int ret = adc_sequence_init_dt(&ntc_adc, &sequence);
	if (ret < 0) {
		return ret;
	}

	sequence.buffer = &raw;
	sequence.buffer_size = sizeof(raw);

	ret = adc_read_dt(&ntc_adc, &sequence);
	if (ret < 0) {
		return ret;
	}

	millivolts = raw;
	return adc_raw_to_millivolts_dt(&ntc_adc, &millivolts);
}
