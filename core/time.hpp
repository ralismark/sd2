/* -*- cpp.doxygen -*- */

#include <chrono>
#include <functional>

namespace rt {

	using clock = std::chrono::steady_clock;

	extern clock::time_point frame_now;

	void exec_at(clock::time_point when, std::function<void()> fn);
	void exec_until(clock::time_point when, std::function<void()> fn);

	void exec_step();

} // namespace rt
