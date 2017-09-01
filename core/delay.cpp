#include "delay.hpp"

namespace rt {

	delay_runner::delay_runner(clock::duration offset)
		: future_offset(clock::duration::zero())
		, current_offset(frame_now + offset)
	{
	}

	rt::clock::time_point delay_runner::when() const
	{
		return current_offset;
	}

	delay_runner& delay_runner::delay(clock::duration d)
	{
		current_offset += d;
		future_offset = clock::duration::zero();
		return *this;
	}

	delay_runner& delay_runner::after()
	{
		return this->delay(future_offset);
	}

	delay_runner& delay_runner::also_exec(std::function<void()> fn)
	{
		exec_at(this->when(), std::move(fn));
		return *this;
	}

	delay_runner& delay_runner::exec(std::function<void()> fn)
	{
		return this->after().also_exec(std::move(fn));
	}

	delay_runner& delay_runner::also_over(clock::duration d, std::function<void(double)> fn)
	{
		auto looper = [start = this->when(), d, f = std::move(fn)] {
			// floating point duration type, since normally it is integer
			// ensures that division works correctly and does not truncate
			using fp_duration = std::chrono::duration<double, clock::duration::period>;
			auto progress = (frame_now - start) / fp_duration(d);
			// ensures that progress is between 0 and 1
			if(progress > 1) {
				progress = 1;
			} else if(progress < 0) {
				progress = 0;
			} else if(!(0 < progress && progress < 1)) {
				// NaN?
				progress = 0;
			}
			f(progress);
		};
		auto start_looper = [lo = std::move(looper), end = this->when() + d] {
			exec_until(end, std::move(lo));
		};
		exec_at(this->when(), std::move(start_looper));
		future_offset = d;
		return *this;
	}

	delay_runner& delay_runner::over(clock::duration d, std::function<void(double)> fn)
	{
		return this->after().also_over(d, std::move(fn));
	}

} // namespace rt
