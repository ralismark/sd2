#pragma once

#include "core/time.hpp"

namespace rt {

	struct delay_runner
	{
	private:
		clock::duration future_offset;
		clock::time_point current_offset;
	public:
		delay_runner(clock::duration offset = clock::duration::zero());

		clock::time_point when() const;
		delay_runner& delay(clock::duration d);
		delay_runner& after();

		delay_runner& also_exec(std::function<void()> fn);
		delay_runner& exec(std::function<void()> fn);

		delay_runner& also_over(clock::duration d, std::function<void(double)> fn);
		delay_runner& over(clock::duration d, std::function<void(double)> fn);
	};

} // namespace rt

