#pragma once

#include <vector>
#include <cstdint>

#include <arm_math.h>

namespace Fir {
class Filter {
public:
	explicit Filter(std::vector<float> const& taps)
		: mTaps(taps), mState(taps.size(), 0.0F)
	{
		if (taps.empty()) {
			return;
		}
		if (taps.size() > UINT16_MAX) {
			mState.clear();
			mTaps.clear();
			return;
		}

		mTapCount = static_cast<uint16_t>(taps.size());
		arm_fir_init_f32(
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

		arm_fir_f32(&mInstance, &unfiltered_value, &result, 1);
		return result;
	}

private:
	std::vector<float> mTaps;
	std::vector<float> mState;
	arm_fir_instance_f32 mInstance{};
	uint16_t mTapCount = 0;
};
}
