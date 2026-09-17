#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

/* A small two-axis One Euro filter. Slow motion gets the strongest jitter
 * rejection; deliberate fast motion raises the cutoff and stays responsive. */
class PointerFilter {
public:
	struct Point {
		float x;
		float y;
	};

	void reset() { ready_ = false; }
	bool ready() const { return ready_; }

	Point update(float x, float y, int64_t time_ns) {
		if (!ready_) {
			ready_ = true;
			last_time_ns_ = time_ns;
			raw_x_ = filtered_x_ = x;
			raw_y_ = filtered_y_ = y;
			dx_ = dy_ = 0.f;
			return {x, y};
		}

		float dt = (float)(time_ns - last_time_ns_) * 1.0e-9f;
		last_time_ns_ = time_ns;
		/* Predicted display timestamps can repeat around session transitions. */
		dt = std::clamp(dt, 1.f / 240.f, 0.1f);

		float derivative_alpha = alpha(kDerivativeCutoffHz, dt);
		dx_ += derivative_alpha * (((x - raw_x_) / dt) - dx_);
		dy_ += derivative_alpha * (((y - raw_y_) / dt) - dy_);
		raw_x_ = x;
		raw_y_ = y;

		float cutoff_x = kMinimumCutoffHz + kSpeedGain * std::fabs(dx_);
		float cutoff_y = kMinimumCutoffHz + kSpeedGain * std::fabs(dy_);
		filtered_x_ += alpha(cutoff_x, dt) * (x - filtered_x_);
		filtered_y_ += alpha(cutoff_y, dt) * (y - filtered_y_);
		return {filtered_x_, filtered_y_};
	}

private:
	static float alpha(float cutoff_hz, float dt) {
		constexpr float pi = 3.14159265358979323846f;
		float tau = 1.f / (2.f * pi * cutoff_hz);
		return dt / (dt + tau);
	}

	/* Balanced preset: about 40 ms of damping at rest and progressively less
	 * lag as hand/controller speed rises. */
	static constexpr float kMinimumCutoffHz = 4.f;
	static constexpr float kSpeedGain = 6.f;
	static constexpr float kDerivativeCutoffHz = 1.f;

	bool ready_ = false;
	int64_t last_time_ns_ = 0;
	float raw_x_ = 0.f, raw_y_ = 0.f;
	float filtered_x_ = 0.f, filtered_y_ = 0.f;
	float dx_ = 0.f, dy_ = 0.f;
};
