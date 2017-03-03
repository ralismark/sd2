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

	keystate(key_type kcode = key_type::Unknown);
	void rebind(key_type kcode = key_type::Unknown);

	bool pressed() const;
	explicit operator bool() const;
	bool operator!() const;

};
