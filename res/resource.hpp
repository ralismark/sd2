/* -*- cpp.doxygen -*- */
#pragma once

#include <map>
#include <type_traits>

namespace res {

	template <typename R>
	class base_controller
	{
	public: // statics
		using resource_type = R;
		using key_type      = typename R::key_type;
	private: // variables
		std::map<key_type, resource_type> contain;
	public: // methods
		resource_type& load(key_type key)
		{
			if(contain.count(key) > 0) {
				contain.emplace(key, key);
			}
			return contain[key];
		}

		void unload(key_type key)
		{
			contain.erase(key);
		}
	};

} // namespace res
