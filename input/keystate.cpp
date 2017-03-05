#include "keystate.hpp"

// class keystate {{{

keystate::keystate(key_type kcode)
	: key(kcode)
{
}

void keystate::rebind(key_type kcode)
{
	key = kcode;
}

bool keystate::pressed() const
{
	return sf::Keyboard::isKeyPressed(key);
}

keystate::operator bool() const
{
	return this->pressed();
}

bool keystate::operator!() const
{
	return !this->pressed();
}

// }}}
