/* -*- cpp.doxygen -*- */
#pragma once

#include <sfml/window/keyboard.hpp>

/**
 * \class keystate
 * \brief Rebindable key state accessor
 *
 * This is a wrapper around sf::Keyboard::isKeyPressed for a bound key. Member
 * functions are provided to simplify code.
 *
 * The main addition of this is that it stores the key being tested. This
 * abstracts away the key, allowing key-independent controls.
 */
class keystate
{
public: // statics
	using key_type = sf::Keyboard::Key;
private: // variables
	key_type key;
public: // methods
	keystate(key_type kcode = key_type::Unknown)
		: key(kcode)
	{
	}

	void rebind(key_type kcode = key_type::Unknown)
	{
		key = kcode;
	}

	bool pressed() const
	{
		return sf::Keyboard::isKeyPressed(key);
	}

	explicit operator bool() const
	{
		return this->pressed();
	}

	bool operator!() const
	{
		return !this->pressed();
	}
};
