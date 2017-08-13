#pragma once

#include "afx.hpp"

void remove_from_board(entityx::Entity e)
{
	if(e.has_component<vec2u>()) {
		tile(*e.component<vec2u>()).piece = {};
		e.remove<vec2u>();
	}
	if(e.has_component<bit_grid>()) {
		e.remove<bit_grid>();
	}
}

struct capturable
	: public piece_interface
{
	virtual ~capturable() = default;

	virtual void on_being_captured(entityx::Entity self, entityx::Entity)
	{
		// remove from board
		remove_from_board(self);

		int change = 10;
		if(self.has_component<piece_data>()) {
			auto type = self.component<piece_data>()->type;
			switch(type) {
			case piece_type::knight:
			case piece_type::bishop:
				change = 30;
				break;
			case piece_type::rook:
				change = 50;
				break;
			case piece_type::queen:
				change = 90;
				break;
			case piece_type::king:
				change = 20;
				break;
			default:
				break;
			}
		}

		var::score += change;
	}
};

bool diff_team(entityx::Entity a, entityx::Entity b)
{
	auto a_data = a.component<piece_data>();
	auto b_data = b.component<piece_data>();
	if(!a_data || !b_data) {
		return false;
	}

	return a_data->team != b_data->team;
}

void move_to(entityx::Entity e, vec2u target)
{
	auto iface = e.component<piece_iface>();
	bool has_iface = iface && *iface;

	if(has_iface) {
		(*iface)->on_move(e, target);
	}

	// remove from original position
	auto pos = e.component<vec2u>();
	if(pos) {
		tile(*pos).piece.invalidate();
	}

	// set the position
	e.assign<vec2u>(target);

	// events and stuff for captures
	auto e_captured = tile(target).piece;
	if(e_captured && e_captured != e && diff_team(e, e_captured)) {
		// target is occupied and can be captured
		if(has_iface) {
			(*iface)->on_capture(e, e_captured);
		}
		auto other_iface = e_captured.component<piece_iface>();
		if(other_iface && *other_iface) {
			(*other_iface)->on_being_captured(e_captured, e);
		}
	}

	refresh_all_moves();

	tile(target).piece = e;
}

void play_hit(); // in main.cpp

rt::delay_runner move_transition(entityx::Entity e, vec2u target, rt::delay_runner run /*= {}*/)
{
	auto from_tile = tile(*e.component<vec2u>()).tile.region();
	auto to_tile = tile(target).tile.region();
	vec2 from = { from_tile.left, from_tile.top };
	vec2 to = { to_tile.left, to_tile.top };

	auto is_alive = std::make_shared<bool>(false);

	return run
		.exec([=] () mutable {
			*is_alive = e.has_component<vec2u>();
			if(*is_alive) {
				e.assign<tag::tile_unbound>();
				move_to(e, target);
			}
		})
		.over(300ms, [=] (double progress) mutable {
			if(*is_alive) {
				auto tween = progress * to + (1 - progress) * from;
				tween.y -= 200*(-progress * progress + progress); // 'jump' animation
				(*e.component<spriteset>())->setPosition(tween);
			}
		})
		.exec([=] () mutable {
			if(is_alive) {
				play_hit();
				e.remove<tag::tile_unbound>();
			}
		});
}

entityx::Entity create_object(const store::resource_type& res)
{
	auto entity = var::world.create();
	entity.assign<spriteset>(res, cfg::sprite_size.x, cfg::sprite_size.y);
	(*entity.component<spriteset>())->setScale(cfg::display_scale, cfg::display_scale);
	return entity;
}

void clear_ai()
{
	var::world.entities.each<tag::ai_control>([] (entityx::Entity e, auto) {
		e.destroy();
	});
	var::world.entities.each<tag::temporary>([] (entityx::Entity e, auto) {
		e.destroy();
	});
}

entityx::Entity create_ai(const store::resource_type& res, piece_type type)
{
	auto piece = create_object(res);
	piece.assign<piece_data>(type, 1);
	piece.assign<piece_iface>(std::make_shared<capturable>());
	piece.assign<tag::ai_control>();
	(*piece.component<spriteset>())->scale(-1, 1);
	(*piece.component<spriteset>())->setOrigin(16, 0);

	return piece;
}

entityx::Entity create_prop(const store::resource_type& res)
{
	auto piece = var::world.create();
	auto& sprite = *piece.assign<spriteset>();
	sprite.load_whole(res.get(), res.size());
	sprite->setScale(cfg::display_scale, cfg::display_scale);
	piece.assign<piece_type>(piece_type::block);
	piece.assign<tag::temporary>();
	return piece;
}

template <typename F>
void fade_and_exec(rt::clock::duration fade_duration, F fn, rt::delay_runner run = {})
{
	run
	.exec([] { ++eff::input_lock; })
	.over(fade_duration, [] (double progress) {
		eff::fade_amount = progress * 255;
	})
	.exec(fn)
	.over(fade_duration, [] (double progress) {
		eff::fade_amount = (1 - progress) * 255;
	})
	.exec([] { --eff::input_lock; });
}

void overlay_color(sf::Color color)
{
	sf::RectangleShape overlay;
	overlay.setFillColor(color);
	overlay.setSize(stdwin.winsize);

	stdwin->draw(overlay);
}

