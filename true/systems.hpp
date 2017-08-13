#pragma once

#include "afx.hpp"

// components:
// vec2u - position on grid. If not present, piece is not on board
// spriteset - set of sprites for object
// piece_iface - custom hooks for piece events
// piece_data - specifies an actual moving piece
// bit_grid - valid moves

namespace tag {

	// position specified by spriteset, not tile
	struct tile_unbound {};

	// controlled by AI
	struct ai_control {};

	// target for AI
	struct ai_target {};

	// remove at end of level
	struct temporary {};

} // namespace tag

namespace sys {

	// updates and draws the spritesets
	struct render_step
		: public entityx::System<render_step>
	{
		void update(entityx::EntityManager& es, entityx::EventManager& events, double dt)
		{
			// draw if sprite has either position or is not bound to tile
			es.each<spriteset>([dt] (entityx::Entity e, spriteset& sprite) {
					bool to_render = false;
					if(!e.has_component<tag::tile_unbound>() && e.has_component<vec2u>()) {
						to_render = true;

						auto square = tile(*e.component<vec2u>()).tile.region();
						sprite->setPosition(square.left, square.top);
					} else if(e.has_component<tag::tile_unbound>()) {
						to_render = true;
					}
					if(to_render) {
						sprite.advance(dt);
						stdwin->draw(*sprite);
					}
				});
		}
	};

	struct update_moves
		: public entityx::System<update_moves>
	{
		void update(entityx::EntityManager& es, entityx::EventManager& events, double dt)
		{
			es.each<vec2u, piece_data>([] (entityx::Entity e, vec2u pos, piece_data& piece) {
					e.assign<bit_grid>(get_valid_moves(pos, piece.team, piece.type));
				});
		}
	};

} // namespace sys

struct entity_world
	: public entityx::EntityX
{
	void load()
	{
		systems.add<sys::render_step>();
		systems.add<sys::update_moves>();
		systems.configure();
	}

	entityx::Entity create()
	{
		return entities.create();
	}

	void refresh_moves()
	{
		systems.update<sys::update_moves>(0);
	}

	void render_step(double dt)
	{
		systems.update<sys::render_step>(dt);
	}
};

namespace var {

	entity_world world;

} // namespace var

void refresh_all_moves()
{
	var::world.refresh_moves();
}
