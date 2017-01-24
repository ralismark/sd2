#pragma once

#include <sfml/window/keyboard.hpp>

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
