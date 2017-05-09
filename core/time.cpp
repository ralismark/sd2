#include "time.hpp"

#include <algorithm>

#include "include/priority_list.hpp"

namespace { // anonymous

	struct exec_store
	{
		rt::clock::time_point when;
		std::function<void()> fn;
	public:
		exec_store(rt::clock::time_point init_when, std::function<void()> init_fn)
			: when(init_when), fn(init_fn)
		{
		}

		friend bool operator<(const exec_store& lhs, const exec_store& rhs)
		{
			return lhs.when < rhs.when;
		}
	};

	priority_list<exec_store> at_queue;
	priority_list<exec_store> until_queue;

} // namespace anonymous

namespace rt {

	clock::time_point frame_now{};

	void exec_at(clock::time_point when, std::function<void()> fn)
	{
		at_queue.emplace(when, std::move(fn));
	}

	void exec_until(clock::time_point when, std::function<void()> fn)
	{
		until_queue.emplace(when, std::move(fn));
	}

	void exec_step()
	{
		auto expired = [] (const exec_store& val) -> bool {
			return val.when < frame_now;
		};

		// process delayed execution
		auto exec_at_end = std::find_if_not(at_queue.begin(), at_queue.end(), expired);
		for(auto it = at_queue.begin(); it != exec_at_end; ++it) {
			it->fn();
		}
		at_queue.erase(at_queue.begin(), exec_at_end);

		// process repetition
		for(auto&& exec : until_queue) {
			exec.fn();
		}
		auto exec_until_end = std::find_if_not(until_queue.begin(), until_queue.end(), expired);
		until_queue.erase(until_queue.begin(), exec_until_end);
	}

} // namespace rt
