#pragma once

#include "afx.hpp"

enum class scene
{
	menu,
	diff,
	play,
	lose,
	map_select,
	pause
} game_state;

// in main.cpp, resets the level
void reset_level(int offset = -1);

struct map_select
{
	char type; // level class
	int level; // level ID in the class
public: // methods
	void load()
	{
		game_state = scene::map_select;
		type = 'a';
		level = 0;
	}

	void unload()
	{
		game_state = scene::play;
	}

	void handle_event(const sf::Event& e)
	{
		if(e.type == sf::Event::KeyPressed) {
			// get all offsets, sorted by class
			auto& offsets = rooms::get_offsets_cache(store::room_txt);
			if(e.key.code == sf::Keyboard::Left) {
				// previous level
				--level;
				if(level < 0) {
					level = offsets[type].size() - 1;
				}
			}
			if(e.key.code == sf::Keyboard::Right) {
				// next level
				++level;
				if(level >= offsets[type].size()) {
					level = 0;
				}
			}
			if(e.key.code == sf::Keyboard::Up || e.key.code == sf::Keyboard::Down) {
				// next or previous class
				// we need to get a list of level classes first
				std::vector<char> types;
				for(auto& entry : offsets) {
					types.push_back(entry.first);
				}
				// find current type
				auto it = std::find(types.begin(), types.end(), type);
				if(e.key.code == sf::Keyboard::Up) {
					// previous class
					if(type == types.front()) {
						type = types.back();
					} else {
						--it;
						type = *it;
					}
				} else {
					// next class
					if(type == types.back()) {
						type = types.front();
					} else {
						++it;
						type = *it;
					}
				}
				// reset level, so we aren't pointing at an invalid one
				level = 0;
			}
			if(e.key.code == sf::Keyboard::Escape) {
				// leave menu without changing level
				this->unload();
			}
			if(e.key.code == sf::Keyboard::Return) {
				// reset the level
				reset_level(offsets[type][level]);
				this->unload();
			}
		}
	}

	void render()
	{
		// message in upper corner
		auto& offsets = rooms::get_offsets_cache(store::room_txt);
		draw_text(fmt::format("Load: {},{}/{}", type, level + 1, offsets[type].size()).c_str(), {0, 20}, sf::Color::White, 15);
	}
};

struct pause_menu
{
	static constexpr std::array<sf::Keyboard::Key, 11> sequence = {{
		sf::Keyboard::Up,
		sf::Keyboard::Up,
		sf::Keyboard::Down,
		sf::Keyboard::Down,
		sf::Keyboard::Left,
		sf::Keyboard::Right,
		sf::Keyboard::Left,
		sf::Keyboard::Right,
		sf::Keyboard::B,
		sf::Keyboard::A,
		sf::Keyboard::Return
	}};
public: // variables
	button resume;
	button restart;
	int secret;
public: // methods
	pause_menu()
		: resume({ 450, 270, 90, 20 })
		, restart({ 450, 240, 90, 20 })
		, secret(0)
	{
	}

	void load()
	{
		game_state = scene::pause;
		var::bg.pause();
	}

	void unload()
	{
		// resume game
		game_state = scene::play;
		if(secret < sequence.size()) {
			// reset secret counter
			secret = 0;
		}
		var::bg.play();
	}

	void handle_event(const sf::Event& e)
	{
		// process resume button events
		auto resume_events = resume.process(e);
		if(contains(resume_events.begin(), resume_events.end(), button::event::release)) {
			this->unload();
		}

		// process restart button event
		auto restart_events = restart.process(e);
		if(contains(restart_events.begin(), restart_events.end(), button::event::release)) {
			this->unload();
			reset_level();
		}

		if(e.type == sf::Event::KeyPressed) {
			if(e.key.code == sf::Keyboard::Escape) {
				this->unload();
			}
			if(secret < sequence.size()) {
				if(e.key.code == sequence[secret]) {
					// advance sequence if key is correct
					++secret;
					std::cout << secret;
				} else {
					// reset it otherwise
					secret = 0;
				}
			}
		}
	}

	void render()
	{
		// fade screen
		overlay_color(sf::Color(0, 0, 0, 150));

		// draw buttons and text
		draw_text("Paused!", { 450, 200 });
		draw_text("restart", { 460, 240 }, sf::Color::White, 20);
		draw_text("continue", { 460, 270 }, sf::Color::White, 20);
		draw_text("(or <Escape>)", { 455, 300 }, sf::Color(200, 200, 200), 15);

		if(secret == sequence.size()) {
			draw_text("Made by Timmy Yao!", {450, 550}, sf::Color::White, 10);
		}
	}
};

struct loss_menu
{
public:
	void load()
	{
		game_state = scene::lose;
		var::world.entities.each<tag::ai_target>([] (entityx::Entity e, auto) {
			// kill player
			remove_from_board(e);
		});
	}

	void unload()
	{
		game_state = scene::play;
		// reset everything
		var::score = 0;
		reset_level();
	}

	void handle_event(const sf::Event& e)
	{
		if(e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Return) {
			this->unload();
		}
	}

	void render()
	{
		overlay_color(sf::Color(0, 0, 0, 100));

		// this is the ending screen
		draw_text("Game over!", {400, 200}, sf::Color::Yellow, 30);
		draw_text("Press <Enter> to restart", {395, 250}, sf::Color::Yellow, 15);
		draw_text(fmt::format("Score: {:04}", var::score).c_str(), {450, 430}, sf::Color::Yellow, 15);
	}
};

struct main_menu
{
	button prev; // is the play button on the main screen
	button next;

	// we're using the background index as the page number
	spriteset background;
public: // methods
	main_menu()
		: prev({ 30, 450, 100, 50 })
		, next({ 30, 510, 100, 50 })
		, background()
	{
	}

	void load()
	{
		game_state = scene::menu;

		// we're not doing this in the constructor due to order or initialisation issues
		background.load(store::menu_png, stdwin.winsize.x, stdwin.winsize.y);
	}

	void unload()
	{
		game_state = scene::diff;
		// start new game
		var::score = 0;
		reset_level();
		var::bg.play();
	}

	void handle_event(const sf::Event& e)
	{
		if(background.index == 0) {
			// main screen
			auto start_ev = prev.process(e);
			auto next_ev = next.process(e);

			if(contains(start_ev.begin(), start_ev.end(), button::event::release)) {
				fade_and_exec(std::chrono::seconds{1}, [this] { this->unload(); });
			}

			if(contains(next_ev.begin(), next_ev.end(), button::event::release)) {
				background.next();
			}
		} else {
			// other help screens
			auto prev_ev = prev.process(e);
			auto next_ev = next.process(e);

			if(contains(prev_ev.begin(), prev_ev.end(), button::event::release)) {
				--background.index;
				background.refresh();
			}

			if(contains(next_ev.begin(), next_ev.end(), button::event::release)) {
				background.next(); // loops around to main screen if at end
			}
		}
	}

	void render()
	{
		stdwin->draw(*background);
	}
};

struct diffselect
{
	button easy;
	button medium;
	button hard;
	button super;
	button ultra;
public: // methods
	diffselect()
		: easy   ({ 100, 200, 200, 50 })
		, medium ({ 100, 250, 200, 50 })
		, hard   ({ 100, 300, 200, 50 })
		, super  ({ 100, 350, 200, 50 })
		, ultra  ({ 100, 450, 200, 50 })
	{
	}

	void unload()
	{
		fade_and_exec(std::chrono::seconds{1}, [] {
				game_state = scene::play;
				var::score = 0;
				reset_level();
				var::bg.play();
			});
	}

	void handle_event(const sf::Event& e)
	{
		auto easy_ev = easy.process(e);
		if(contains(easy_ev.begin(), easy_ev.end(), button::event::release)) {
			var::score_bonus = 0;
			this->unload();
		}

		auto medium_ev = medium.process(e);
		if(contains(medium_ev.begin(), medium_ev.end(), button::event::release)) {
			var::score_bonus = 300;
			this->unload();
		}

		auto hard_ev = hard.process(e);
		if(contains(hard_ev.begin(), hard_ev.end(), button::event::release)) {
			var::score_bonus = 800;
			this->unload();
		}

		auto super_ev = super.process(e);
		if(contains(super_ev.begin(), super_ev.end(), button::event::release)) {
			var::score_bonus = 1000;
			this->unload();
		}

		auto ultra_ev = ultra.process(e);
		if(contains(ultra_ev.begin(), ultra_ev.end(), button::event::release)) {
			var::score_bonus = 1500;
			this->unload();
		}
	}

	void render()
	{
		for(int i = 200; i <= 350; i += 50) {
			sf::RectangleShape button({200, 40});
			button.setPosition(100, i);
			button.setFillColor(sf::Color(255, 126, 40)); // start button colour
			stdwin->draw(button);
		}

		draw_text("Select difficulty:", {100, 100}, sf::Color::Yellow, 30);
		draw_text("Easy",   {100, 200});
		draw_text("Medium", {100, 250});
		draw_text("Hard",   {100, 300});
		draw_text("Super",  {100, 350});
		draw_text("???",    {100, 450});
	}
};
