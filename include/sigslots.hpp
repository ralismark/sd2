/* -*- cpp.doxygen -*- */
#pragma once

#include "priority_list.hpp"
#include <functional>

/**
 * \class signal
 * \brief Callback container and emitter
 *
 * This uses the signal/slots design pattern (look on wikipedia) to allow
 * attaching multiple callbacks to events. Provide the arguments for callbacks
 * as the template parameters.
 */
template <typename... Args>
class signal
{
	using function_signature = void(Args...);
	using slot_target = std::pair<std::function<function_signature>, int>;

	static bool slot_less_cmp(const slot_target& lhs, const slot_target& rhs)
	{
		return lhs.second < rhs.second;
	}

private: // internal statics

	using function_type  = std::function<function_signature>;
	using container_type = priority_list<slot_target, decltype(slot_less_cmp)>;
	using slot_id        = typename container_type::const_iterator;

private: // variables

	container_type slotlist;

public: // methods

	signal()
		: slotlist(&slot_less_cmp)
	{
	}

	slot_id connect(function_type fn, int priority = 0)
	{
		return slotlist.emplace(std::move(fn), priority);
	}

	void disconnect(slot_id slot)
	{
		slotlist.erase(slot);
	}

	void disconnect_all()
	{
		slotlist.clear();
	}

	void emit(Args... args)
	{
		for(auto& slot : slotlist) {
			slot.first(args...);
		}
	}

	void operator()(Args... args)
	{
		this->emit(std::forward<Args>(args)...);
	}

	slot_id operator+=(function_type fn)
	{
		return this->connect(std::move(fn));
	}

};
