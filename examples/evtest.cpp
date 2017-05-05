#include "core/runtime.cpp"

#include <iostream>
#include "include/fmt.hpp"

const char* ev_name_map[sf::Event::EventType::Count] = {
	"closed",
	"resized",
	"lost focus",
	"gained focus",
	"text entered",
	"key pressed",
	"key released",
	"mouse wheel moved",
	"mouse wheel scrolled",
	"mouse button pressed",
	"mouse button released",
	"mouse moved",
	"mouse entered",
	"mouse left",
	"joystick button pressed",
	"joystick button released",
	"joystick moved",
	"joystick connected",
	"joystick disconnected",
	"touch began",
	"touch moved",
	"touch ended",
	"sensor changed",
};

void event(const sf::Event& e)
{
	fmt::print("{}: {}\n", rt::frame, ev_name_map[e.type]);

	switch(e.type) {
	case sf::Event::Resized:
		fmt::print("\t| size\n"
		           "\t| width: {}\n"
		           "\t| height: {}\n",
		           e.size.width, e.size.height);
		break;

	case sf::Event::KeyPressed:
	case sf::Event::KeyReleased:
		fmt::print("\t| key\n"
		           "\t| code: {}\n"
		           "\t| alt: {}\n"
		           "\t| control: {}\n"
		           "\t| shift: {}\n"
		           "\t| system: {}\n",
		           e.key.code, e.key.alt, e.key.control, e.key.shift,
		           e.key.system);
		break;

	case sf::Event::TextEntered:
		fmt::print("\t| text\n"
		           "\t| unicode: {}\n",
		           e.text.unicode);
		break;

	case sf::Event::MouseMoved:
		fmt::print("\t| mouseMove\n"
		           "\t| x: {}\n"
		           "\t| y: {}\n",
		           e.mouseMove.x, e.mouseMove.y);
		break;

	case sf::Event::MouseButtonPressed:
	case sf::Event::MouseButtonReleased:
		fmt::print("\t| mouseButton\n"
		           "\t| button: {}\n"
		           "\t| x: {}\n"
		           "\t| y: {}\n",
		           e.mouseButton.button, e.mouseButton.x,
		           e.mouseButton.y);
		break;

	case sf::Event::MouseWheelMoved:
		fmt::print("\t| mouseWheel\n"
		           "\t| delta: {}\n"
		           "\t| x: {}\n"
		           "\t| y: {}\n",
		           e.mouseWheel.delta, e.mouseWheel.x, e.mouseWheel.y);
		break;

	default:
		fmt::print("\t| no data\n");
		break;
	}
}

void initial()
{
	rt::on_win_event.connect(event);
}
