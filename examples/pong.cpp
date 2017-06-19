#include "core/runtime.cpp"
#include "include/vector.hpp"
#include "input/keystate.hpp"
#include "include/randutils.hpp"
#include "include/fmt.hpp"
#include "include/entityx.hpp"
#include "res0.hpp"

#include <sfml/graphics.hpp>

#include <string>
#include <numeric>
#include <cassert>
#include <deque>

randutils::mt19937_rng rng;

template <typename T>
constexpr T clamp(const T& val, const T& low, const T& hi)
{
	return val < low ? low : val > hi ? hi : val;
}

namespace cfg {

	double time_scale = 1;
	double move_scale = 400;

	double player_width = 10;
	double player_margin = 10;
	double player_size = 200;

	size_t tick_window = 100;

} // namespace cfg

namespace var {

	size_t left_score = 0;
	size_t right_score = 0;

} // namespace var

struct player
{ // {{{
	enum class team {
		left, right
	} side;
	keystate up, down;
	double loc;

	player(team s, keystate::key_type u, keystate::key_type d)
		: side(s), up(u), down(d), loc(0)
	{
	}
}; // }}}

struct ball
{ // {{{
	double diameter;
	vec2 loc;
	vec2 vel;

	ball(double diam)
		: diameter(diam), loc(), vel()
	{
		this->reset();
	}

	void reset()
	{
		loc = vec2{(stdwin.winsize.x - diameter) / 2, (stdwin.winsize.y - diameter) / 2};
		vel = vec2{0, 0};
	}
}; // }}}

struct input_sys
	: public entityx::System<input_sys>
{ // {{{
	void update(entityx::EntityManager& es, entityx::EventManager &events, double dt)
	{
		auto per = [&] (player& p) {
			if(p.up && !p.down) {
				p.loc -= cfg::move_scale * dt;
			}
			if(!p.up && p.down) {
				p.loc += cfg::move_scale * dt;
			}

			p.loc = clamp(p.loc, 0.0, stdwin.winsize.y - cfg::player_size);
		};
		es.each<player>([&] (entityx::Entity e, player& p) { per(p); });
	}
}; // }}}

struct render_sys
	: public entityx::System<render_sys>
{ // {{{
	void update(entityx::EntityManager& es, entityx::EventManager &events, double /* dt */)
	{
		sf::RectangleShape rect;
		rect.setFillColor(sf::Color::White);

		rect.setSize(sf::Vector2f(float(cfg::player_width), float(cfg::player_size)));
		auto render_player = [&] (const player& p) {
			double root_x = 0;
			switch(p.side) {
			case player::team::left:
				root_x = cfg::player_margin;
				break;
			case player::team::right:
				root_x = stdwin.winsize.x - cfg::player_margin - cfg::player_width;
				break;
			default:
				assert(false && "unhandled enum switch case");
				break;
			}
			rect.setPosition(float(root_x), float(p.loc));

			stdwin->draw(rect);
		};
		es.each<player>([&] (entityx::Entity e, const player& p) { render_player(p); });

		auto render_ball = [&] (const ball& b) {
			rect.setSize(sf::Vector2f(b.diameter, b.diameter));
			rect.setPosition(float(b.loc.x), float(b.loc.y));

			stdwin->draw(rect);
		};
		es.each<ball>([&] (entityx::Entity e, const ball& b) { render_ball(b); });
	}
}; // }}}

struct ball_move_sys
	: public entityx::System<ball_move_sys>
{ // {{{
	void update(entityx::EntityManager& es, entityx::EventManager &events, double dt)
	{
		auto move_ball = [&] (ball& b) {
			b.loc += b.vel * dt;

			if(b.loc.y + b.diameter > stdwin.winsize.y
			   || b.loc.y < 0) {
				b.vel.y *= -1;
				b.loc.y = clamp(b.loc.y, 0.0, stdwin.winsize.y - b.diameter);
			}
		};

		es.each<ball>([&] (entityx::Entity e, ball& b) { move_ball(b); });
	}
}; // }}}

struct ball_hit_sys
	: public entityx::System<ball_hit_sys>, public entityx::Receiver<ball_hit_sys>
{ // {{{
	struct lose_event {
		const player* p;
		ball* b;
	};

	void configure(entityx::EventManager& em)
	{
		em.subscribe<lose_event>(*this);
	}

	void update(entityx::EntityManager& es, entityx::EventManager &events, double /* dt */)
	{
		auto decoll = [&] (const player& p, ball& b) {
			if(b.loc.y + b.diameter > p.loc
			   && b.loc.y < p.loc + cfg::player_size) {
				// colliding
				b.vel.x *= -1;
				if(p.up && !p.down) {
					b.vel.y -= cfg::move_scale / 10;
				}
				if(!p.up && p.down) {
					b.vel.y += cfg::move_scale / 10;
				}
				return;
			}
			events.emit<lose_event>(lose_event{&p, &b});
		};

		auto decoll_general = [&] (const player& p, ball& b) {
			auto hit_space = cfg::player_width + cfg::player_margin;
			bool contact = false;
			switch(p.side) {
			case player::team::left:
				contact = b.loc.x < hit_space;
				break;
			case player::team::right:
				contact = b.loc.x + b.diameter > stdwin.winsize.x - hit_space;
				break;
			default:
				assert(false && "unhandled enum switch case");
				break;
			}
			if(contact) {
				b.loc.x = clamp(b.loc.x, hit_space, stdwin.winsize.x - b.diameter - hit_space);
				b.vel *= 1.05;
				decoll(p, b);
			}
		};

		es.each<ball>([&] (entityx::Entity eb, ball& b) {
				es.each<player>([&] (entityx::Entity ep, const player& p) {
						decoll_general(p, b);
					});
			});
	}

	void receive(const lose_event& loss)
	{
		assert(loss.p && loss.b && "should not be null");
		switch(loss.p->side) {
		case player::team::left:
			++var::right_score;
			break;
		case player::team::right:
			++var::left_score;
			break;
		}
		loss.b->reset();
	}
}; // }}}

struct world
	: public entityx::EntityX
{ // {{{
	entityx::Entity make_ball()
	{
		auto b = entities.create();
		b.assign<ball>(10);
		return b;
	}

	void load()
	{
		systems.add<ball_hit_sys>();
		systems.add<ball_move_sys>();
		systems.add<input_sys>();
		systems.add<render_sys>();
		systems.configure();

		auto left = entities.create();
		left.assign<player>(player::team::left, sf::Keyboard::W, sf::Keyboard::S);

		auto right = entities.create();
		right.assign<player>(player::team::right, sf::Keyboard::I, sf::Keyboard::K);

		make_ball();
	}

	void update(double dt)
	{
		dt *= cfg::time_scale;

		systems.update<input_sys>(dt);
		systems.update<ball_move_sys>(dt);
		systems.update<ball_hit_sys>(dt);
		systems.update<render_sys>(dt);
	}
}; // }}}

namespace var {

	world world;
	sf::Clock clock;
	std::deque<double> past_ticks;

	sf::Font monofonto;
	sf::Text fps_counter;
	sf::Text score;

} // namespace var

double get_fps()
{
	double tick_sum = std::accumulate(var::past_ticks.begin(), var::past_ticks.end(), 0.0);
	double avg_tick = tick_sum / var::past_ticks.size();
	return 1 / avg_tick;
}

void start_ball()
{
	bool once = false;
	auto each = [&] (ball& b) {
		if(!once && b.vel.x == 0 && b.vel.y == 0) {
			once = true;
			b.vel = 200 * vec2{ rng.uniform(0, 1) * 2 - 1, rng.uniform(0, 1) * 2 - 1 };
		}
	};
	var::world.entities.each<ball>([&] (entityx::Entity e, ball& b) { each(b); });

	if(!once) {
		each(*var::world.make_ball().component<ball>());
	}
}

void events(const sf::Event& e)
{
	if(e.type == sf::Event::KeyPressed) {
		if(e.key.code == sf::Keyboard::Space) {
			start_ball();
		}
	}
}

void initial()
{
	cfg::tick_window = stdwin.winfps < 1 ? 200 : stdwin.winfps / 2;

	rt::on_frame.connect([] {
			auto elapsed = var::clock.restart();
			var::past_ticks.push_front(elapsed.asSeconds());
			if(var::past_ticks.size() > cfg::tick_window) {
				var::past_ticks.pop_back();
			}

			var::fps_counter.setString(std::to_string(int(get_fps())).c_str());
			var::score.setString(fmt::format("{:>3} : {:<3}", var::left_score, var::right_score).c_str());
			var::score.setPosition((stdwin.winsize.x - var::score.getLocalBounds().width) / 2, 20);
		}, 0);
	rt::on_frame.connect([] {
			stdwin->clear(sf::Color::Black);

			stdwin->draw(var::score);
			var::world.update(var::past_ticks.front());
			stdwin->draw(var::fps_counter);

			stdwin->display();
		}, 30);
	rt::on_win_event.connect(events);

	if(!var::monofonto.loadFromMemory(store::monofonto_ttf.get(), store::monofonto_ttf.size())) {
		std::cout << "load err: monofonto.ttf";
		rt::exit(1);
	}

	var::fps_counter.setPosition(10, stdwin.winsize.y - 10 - var::monofonto.getLineSpacing(16));
	var::fps_counter.setFont(var::monofonto);
	var::fps_counter.setCharacterSize(16);
	var::fps_counter.setFillColor(sf::Color{100, 100, 100});

	var::score.setFont(var::monofonto);
	var::score.setCharacterSize(30);
	var::score.setFillColor(sf::Color::White);

	var::world.load();
	var::clock.restart();
}
