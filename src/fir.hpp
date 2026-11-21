#pragma once

#include <vector>
#include <cstdint>

#include <arm_math.h>

namespace Fir {
class Filter {
public:
	explicit Filter(std::vector<float> const& taps)
		: mTaps(taps.size(), 0), mState(taps.size(), 0)
	{
		if (taps.empty()) {
			return;
		}
		if (taps.size() > UINT16_MAX) {
			mState.clear();
			mTaps.clear();
			return;
		}

		for (std::size_t i = 0; i < taps.size(); ++i) {
			arm_float_to_q31(&taps[i], &mTaps[i], 1);
		}
		mTapCount = static_cast<uint16_t>(taps.size());
		arm_fir_init_q31(
			&mInstance,
			mTapCount,
			mTaps.data(),
			mState.data(),
			1);
	}

	float filter(float unfiltered_value)
	{
		float result = 0.0F;
		if (mTapCount == 0) {
			return unfiltered_value;
		}
		unfiltered_value /= 32768.0F;
		q31_t un_fil = 0;
		arm_float_to_q31(&unfiltered_value, &un_fil, 1);
		arm_fir_fast_q31(&mInstance, &un_fil, &un_fil, 1);
		arm_q31_to_float(&un_fil, &result, 1);
		result *= 32768.0F;
		return result;
	}

private:
	std::vector<q31_t> mTaps;
	std::vector<q31_t> mState;
	arm_fir_instance_q31 mInstance{};
	uint16_t mTapCount = 0;
};
}
