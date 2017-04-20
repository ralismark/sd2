#pragma once

#include "depends/velm/include/velm.hpp"

// these are conversions to sf::vector, which allow velm::vector to be used
// semi-transparently with the SFML API

#include <sfml/system/vector2.hpp>
#include <sfml/system/vector3.hpp>

namespace velm { namespace usr {

	template <typename T>
	struct tie<sf::Vector2<T>>
	{
		auto operator()(const sf::Vector2<T>&& val) const
		{
			return std::make_tuple(val.x, val.y);
		}

		auto operator()(const sf::Vector2<T>& val) const
		{
			return std::tie(val.x, val.y);
		}

		auto operator()(sf::Vector2<T>& val) const
		{
			return std::tie(val.x, val.y);
		}
	};

	template <typename T>
	struct converter_to<sf::Vector2<T>>
	{
		template <typename... Ts>
		sf::Vector2<T> operator()(Ts&&... args) const
		{
			return sf::Vector2<T>{std::forward<Ts>(args)...};
		}
	};


	template <typename T>
	struct tie<sf::Vector3<T>>
	{
		auto operator()(const sf::Vector3<T>&& val) const
		{
			return std::make_tuple(val.x, val.y, val.z);
		}

		auto operator()(const sf::Vector3<T>& val) const
		{
			return std::tie(val.x, val.y, val.z);
		}

		auto operator()(sf::Vector3<T>& val) const
		{
			return std::tie(val.x, val.y, val.z);
		}
	};

	template <typename T>
	struct converter_to<sf::Vector3<T>>
	{
		template <typename... Ts>
		sf::Vector3<T> operator()(Ts&&... args) const
		{
			return sf::Vector3<T>{std::forward<Ts>(args)...};
		}
	};

} } // namespace velm::usr

template <typename T, size_t N>
using vector = velm::vector<T, N>;

using vec2  = vector<double,       2>;
using vec2i = vector<int,          2>;
using vec2u = vector<unsigned int, 2>;

using vec3  = vector<double,       3>;
using vec3i = vector<int,          3>;
using vec3u = vector<unsigned int, 3>;

