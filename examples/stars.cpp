#include "core/runtime.cpp"
#include "include/fmt.hpp"
#include "include/randutils.hpp"
#include "include/vector.hpp"
#include "include/entityx.hpp"

#include <SFML/Graphics.hpp>

randutils::mt19937_rng rng;

namespace cfg {

	double time_scale = 10.0;
	double colour_scale = 10.0;

}

vec3 reposition()
{
	auto extent = stdwin.winsize;

	vec3 pos;
	pos.x = rng.variate<double, std::normal_distribution>(0, extent.x);
	pos.y = rng.variate<double, std::normal_distribution>(0, extent.y);
	pos.z = rng.uniform(0.0, 100.0);

	return pos;
}

struct move_sys
	: public entityx::System<move_sys>
{
	void update(entityx::EntityManager& es, entityx::EventManager &events, double dt)
	{
		es.each<vec3>([dt] (entityx::Entity e, vec3& pos) {
				pos.z -= 1 * dt;
				if(pos.z < 0) {
					pos = reposition();
				}
			});
	}
};

struct render_sys
	: public entityx::System<render_sys>
{
	void update(entityx::EntityManager& es, entityx::EventManager &events, double)
	{
		stdwin->clear(sf::Color::Black);

		static sf::RectangleShape shape = [] {
			sf::RectangleShape r;
			r.setOrigin(float(-stdwin.winsize.x / 2), float(-stdwin.winsize.y / 2));
			r.setSize(sf::Vector2f(1, 1));
			return r;
		}();

		es.each<vec3>([&] (entityx::Entity e, vec3& pos) {
				unsigned char value = static_cast<unsigned char>(255 * cfg::colour_scale / (pos.z + cfg::colour_scale));
				shape.setFillColor(sf::Color(value, value, value));

				vec2 view_pos = 10 * pos.xy / pos.z;
				shape.setPosition(view_pos.x, view_pos.y);

				stdwin->draw(shape);
			});

		stdwin->display();
	}
};

struct level
	: public entityx::EntityX
{
	void load()
	{
		systems.add<move_sys>();
		systems.add<render_sys>();
		systems.configure();

		int star_count = 1000;
		while(star_count --> 0) {
			auto entity = entities.create();
			entity.assign<vec3>(reposition());
		}
	}

	void update(double dt)
	{
		dt *= cfg::time_scale;
		systems.update<move_sys>(dt);
		systems.update<render_sys>(dt);
	}
};

namespace var {

	level world;
	sf::Clock clock;

} // namespace var

void initial()
{
	rt::on_frame.connect([] {
			auto elapsed = var::clock.restart();
			var::world.update(elapsed.asSeconds());
		});

	var::world.load();
	var::clock.restart();

	auto parse_double = [] (const std::string& s) {
		try {
			size_t idx;
			double ret = std::stod(s, &idx);
			if(ret <= 0 || idx < s.size()) {
				throw 0; // any type will do
			}
			return ret;
		} catch(...) {
			fmt::print("{}: {}: {}\n", rt::pgname, s, "invalid format");
			rt::exit(1);
		}
		return 0.0;
	};

	if(rt::opt::a) {
		// a: the color multiplier
		cfg::colour_scale = parse_double(*rt::opt::a);
	}

	if(rt::opt::b) {
		cfg::time_scale = parse_double(*rt::opt::b);
	}
}
