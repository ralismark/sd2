#include "afx.hpp"

#include "spriteset.hpp"
#include "grid_tile.hpp"
#include "grid_iface.hpp"
#include "grid.hpp"
#include "piece.hpp"
#include "valid_moves.hpp"
#include "systems.hpp"
#include "ai.hpp"
#include "text.hpp"
#include "utils.hpp"
#include "rooms.hpp"
#include "menus.hpp"

randutils::default_rng rng;

namespace var {

	int score = 0;

	sf::Clock clock;

	entityx::Entity exit;
	entityx::Entity knight;

	pause_menu pause;
	loss_menu lose;
	map_select select_menu;
	main_menu mainmenu;

	sf::Music hit;
	sf::Music bg;

} // namespace var

namespace eff { // effects

	int input_lock = 0; // locks input controls
	int fade_amount = 0; // fade screen, 0-255

} // namespace eff

void post_effects()
{
	draw_text(fmt::format("{:04}", var::score).c_str(), {0, 0}, sf::Color::Yellow, 15);

	overlay_color(sf::Color(0, 0, 0, eff::fade_amount));
}

void play_hit()
{
	var::hit.play();
}

void reset_level(int offset /*= -1*/)
{
	rooms::load_level(store::room_txt, var::knight, var::exit, offset);
	var::world.refresh_moves();
}

void advance_stage()
{
	var::score += 100;
	fade_and_exec(500ms, [] { reset_level(); });
}

bool move_ai = false;

void win_events(const sf::Event& e)
{
	// input lock stops all events (e.g. for animations)
	if(eff::input_lock) {
		return;
	}

	if(game_state == scene::menu) {
		var::mainmenu.handle_event(e);
		return;
	} else if(game_state == scene::pause) {
		var::pause.handle_event(e);
		return;
	} else if(game_state == scene::lose) {
		var::lose.handle_event(e);
		return;
	} else if(game_state == scene::map_select) {
		var::select_menu.handle_event(e);
	} else {
		if(e.type == sf::Event::KeyPressed) {
			switch(e.key.code) {
			case sf::Keyboard::F12:
				cfg::weight_overlay = !cfg::weight_overlay;
				break;
			case sf::Keyboard::F11:
				cfg::danger_overlay = !cfg::danger_overlay;
				break;
			case sf::Keyboard::F10:
				var::select_menu.load();
				break;
			case sf::Keyboard::Escape:
				var::pause.load();
				break;
			default:
				break;
			}
		}
	}

	auto events = var::grid.dispatch_event(e);
	for(int x = 0; x < cfg::width; ++x) {
		for(int y = 0; y < cfg::height; ++y) {
			if(!contains(events[x][y].begin(), events[x][y].end(), button::event::release)) {
				continue; // not this tile
			}

			auto exit_pos = *var::exit.component<vec2u>();
			bool at_exit = x == exit_pos.x && y == exit_pos.y;

			if(ensure_bitgrid(var::knight)[square_to_index({x, y})]) {
				auto run = move_transition(var::knight, {x, y});
				if(at_exit) {
					advance_stage();
				} else {
					run.exec([] { move_ai = true; });
				}
			}
		}
	}
}

// gets all tiles movable to by enemies
bit_grid get_target_tiles()
{
	bit_grid out;
	var::world.entities.each<bit_grid, vec2u>([&] (entityx::Entity e, bit_grid& moves, vec2u&) {
			if(e != var::knight) {
				out |= moves;
			}
		});
	return out;
}

// get the highest AI move weight of each tile
// AI implemented in ai.hpp
grid_of<int> get_ai_weights()
{
	grid_of<int> out = {};
	var::world.entities.each<tag::ai_control, vec2u>([&] (entityx::Entity e, auto, auto) {
		auto scores = get_overall_weights(e);
		for(auto& move : scores) {
			auto& tile = out[move.first.first][move.first.second];
			tile = tile > move.second ? tile : move.second;
		}
	});

	return out;
}

// executed once per frame
void full_cycle(double dt)
{
	if(game_state == scene::menu) {
		var::mainmenu.render();
		overlay_color(sf::Color(0, 0, 0, eff::fade_amount)); // hide score
		return; // override everything
	}

	// move_ai used to determine when to start moving the ai

	// normally this would be done with the event runner, but currently
	// there's a bug in that. This is a workaround until that is fixed
	if(move_ai) {
		move_all_ai();
		move_ai = false;
	}

	stdwin->clear(sf::Color(0,0,0));

	auto& moves = ensure_bitgrid(var::knight);
	auto danger_tiles = get_target_tiles();

	// weights only displayed if not zero, default init will result in none being shown
	grid_of<int> weights = {};
	if(cfg::weight_overlay) {
		weights = get_ai_weights();
	}

	for(int x = 0; x < cfg::width; ++x) {
		for(int y = 0; y < cfg::height; ++y) {
			auto& current_tile = tile(x, y);

			bool is_dark = (x ^ y) & 1;
			auto color = is_dark ? sf::Color(50, 50, 50) : sf::Color(70, 70, 70);

			if(moves[square_to_index({x, y})]) {
				color.g = 100;
			}
			// debug overlay stuff toggled by F11 and F12
			if(cfg::danger_overlay && danger_tiles[square_to_index({x, y})]) {
				color.r = 100;
			}
			if(current_tile.tile.current_state() == button::state::active) {
				color.b = 100;
			}
			current_tile.draw(color);

			if(weights[x][y] != 0) {
				auto region = current_tile.tile.region();
				draw_text(std::to_string(weights[x][y]).c_str(), {region.left, region.top}, sf::Color::White, 15);
			}

		}
	}

	// entities do not animate when paused
	// otherwise (e.g. lose screen), they do
	if(game_state == scene::pause) {
		var::world.render_step(0);
	} else {
		var::world.render_step(dt);
	}

	// render the appropriate scenes
	switch(game_state) {
	case scene::pause:
		var::pause.render();
		break;
	case scene::lose:
		var::lose.render();
		break;
	case scene::map_select:
		var::select_menu.render();
		break;
	default:
		break;
	}

	// overlays and stuff
	post_effects();
}

// callback for when the player is captured
// makes them lose
struct player_effects
	: public piece_interface
{
	virtual ~player_effects() = default;

	virtual void on_being_captured(entityx::Entity, entityx::Entity)
	{
		var::lose.load();
	}
};

void initial()
{
	// no key repeat
	stdwin->setKeyRepeatEnabled(false);

	rt::on_frame.connect([] {
		full_cycle(var::clock.restart().asSeconds() * cfg::time_scale);
		stdwin->display();
		});

	rt::on_win_event.connect(win_events);

	// we're showing the main menu first, but we initialise everything at the beginning
	// it was much easier to add on a menu doing it this way than to load everything when we actually start
	var::mainmenu.load();

	// load music
	var::hit.openFromMemory(store::hit_wav.get(), store::hit_wav.size());
	var::hit.setVolume(50);
	var::hit.setPitch(0.5);

	var::bg.openFromMemory(store::bg_wav.get(), store::bg_wav.size());
	var::bg.setLoop(true);
	var::bg.setVolume(20);

	// initialise entity systems
	var::world.load();

	// load grid
	var::grid.size = cfg::sprite_size * cfg::display_scale;
	var::grid.origin = stdwin.winsize / 2 - vec2(cfg::width, cfg::height) / 2 * var::grid.size;
	var::grid.init();

	// load exit
	var::exit = create_object(store::stair_png);

	// load knight player
	var::knight = create_object(store::knight_png);
	var::knight.assign<piece_data>(piece_type::knight);
	var::knight.assign<tag::ai_target>();
	var::knight.assign<piece_iface>(std::make_shared<player_effects>());

	// load level
	reset_level();
}
