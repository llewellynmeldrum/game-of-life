#pragma once
#include <chrono>
struct CLOCK {
	using clock_t = std::chrono::steady_clock;
	using time_point = std::chrono::time_point<clock_t>;
	using duration = std::chrono::duration<double>;
	using sec = std::chrono::seconds;
	using msec = std::chrono::milliseconds;
	using usec = std::chrono::microseconds;
	using nsec = std::chrono::nanoseconds;

	inline void start() noexcept {
		t0 = now();
	}
	inline void stop() noexcept {
		t1 = now();
	}



	inline time_point restart() noexcept {
		return clock_t::now();
	}


	inline double get_s_between() noexcept {
		return (get_ns_between() / 1'000'000'000.0);
	}
	inline double get_ms_between() noexcept {
		return (get_ns_between() / 1'000'000.0);
	}
	inline double get_us_between() noexcept {
		return (get_ns_between() / 1'000.0);
	}
	inline size_t get_ns_between() noexcept {
		auto elapsed_duration = get_duration(t0, t1);
		return std::chrono::duration_cast<nsec>(elapsed_duration).count();
	}

	inline double s_since_start() noexcept {
		return (ns_since_start() / 1'000'000'000.0);
	}
	inline double ms_since_start() noexcept {
		return (ns_since_start() / 1'000'000.0);
	}
	inline double us_since_start() noexcept {
		return (ns_since_start() / 1000.0);
	}
	inline size_t ns_since_start() noexcept {
		auto elapsed_duration = get_duration(t0, now());
		auto elapsed_ms = std::chrono::duration_cast<nsec>(elapsed_duration).count();
		return elapsed_ms;
	}
  private:
	time_point t0;
	time_point t1;

	inline duration get_duration(time_point t0, time_point t1) {
		return t1 - t0;
	}
	inline time_point now() noexcept {
		return clock_t::now();
	}
};

