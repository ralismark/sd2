#include "core/runtime.cpp"
#include "include/randutils.hpp"
#include "include/vector.hpp"
#include "include/win32.hpp"

#include <sfml/graphics.hpp>

#include <vector>

constexpr size_t star_count = 1000;
constexpr double max_z = 100;

namespace central {

	randutils::mt19937_rng rng;

	struct star_point {
		vec3 pos;
		sf::RectangleShape shape;

		star_point()
			: pos(), shape()
		{
			shape.setOrigin(float(-stdwin.winsize.x / 2), float(-stdwin.winsize.y / 2));
			shape.setSize(sf::Vector2f(1, 1));

			this->reposition();
			this->update_shape();
		}

		void reposition()
		{
			auto extent = stdwin.winsize;

			pos.x = rng.variate<double, std::normal_distribution>(0, extent.x);
			pos.y = rng.variate<double, std::normal_distribution>(0, extent.y);
			pos.z = rng.uniform(0.0, max_z);
		}

		void update_shape()
		{
			auto value = (unsigned char)((pos.z > max_z ? 0 : (1 - (pos.z / max_z))) * 255);
			shape.setFillColor(sf::Color(value, value, value));

			vec2 view_pos(10 * pos.x / pos.z, 10 * pos.y / pos.z);
			shape.setPosition(float(view_pos.x), float(view_pos.y));
		}
	};

	std::vector<star_point> stars;

	void update()
	{
		for(auto& star : stars) {
			// move
			if((star.pos.z -= 1.0) < 0) {
				star.reposition();
			}

			star.update_shape();
		}
	}

	void render()
	{
		stdwin->clear(sf::Color::Black);

		for(auto& star : stars) {
			stdwin->draw(star.shape);
		}

		stdwin->display();
	}

	void var_init()
	{
		stars.resize(star_count);
	}

	void move_exit(const sf::Event& e)
	{
		if(rt::frame < 10) {
			return;
		}

		switch(e.type) {
		case sf::Event::KeyPressed:
		case sf::Event::MouseMoved:
		case sf::Event::MouseButtonPressed:
		case sf::Event::MouseWheelMoved:
		case sf::Event::MouseWheelScrolled:
			// all of these close the program
			rt::exit(0);
			break;
		default:
			break; // do nothing
		}
	}

}

vec2i get_monitor_size()
{
	return vec2i{GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
}

bool is_screensaver()
{
	// based on extension
	char fname[MAX_PATH + 1];
	if(GetModuleFileName(nullptr, fname, MAX_PATH + 1) == 0) {
		return false;
	}
	std::string fname_str = fname;
	auto dot_place = fname_str.find_last_of('.');
	if(dot_place == std::string::npos
	   || dot_place == fname_str.size()
	   || fname_str.substr(dot_place + 1) != "scr") {
		return false;
	}
	return true;
}

void initial()
{
	rt::on_frame.connect(central::update, 20);
	rt::on_frame.connect(central::render, 30);

	if(rt::opt::a || is_screensaver()) {
		rt::on_win_event.connect(central::move_exit);

		stdwindow::winstyle = sf::Style::Fullscreen;
		stdwindow::winsize = get_monitor_size();

		stdwin.init();
		stdwin->setMouseCursorVisible(false);
	}

	central::var_init();
}
