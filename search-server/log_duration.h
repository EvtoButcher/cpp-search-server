#pragma once
#include <chrono>
#include <iostream>

#define LOG_DURATION(x) LogDuration time(x)

class LogDuration
{
public:
	using Clock = std::chrono::steady_clock;
	LogDuration(std::string name)
		:start_time(Clock::now())
		, duration_name(name) {
	}

	~LogDuration() {
		using namespace std::literals;

		const auto end_time = Clock::now();
		const auto dur = end_time - start_time;
		std::cerr << duration_name << ": "s << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() << " ms" << std::endl;
	}

private:
	const Clock::time_point start_time;
	const std::string duration_name;
};

