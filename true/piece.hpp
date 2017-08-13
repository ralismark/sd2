#pragma once

#include "afx.hpp"

// types of pieces, used for movement
enum class piece_type {
	block = 0,
	knight,
	bishop,
	rook,
	queen,
	king,
	pawn
};

// also for movement, represents what tiles can be moved to
using bit_grid = std::bitset<cfg::width * cfg::height>;

// inherit and overload for specific hooks
struct piece_interface
{
	virtual ~piece_interface() = default;

	virtual void on_move(entityx::Entity, vec2u target) {}
	virtual void on_capture(entityx::Entity, entityx::Entity target) {}
	virtual void on_being_captured(entityx::Entity, entityx::Entity capturer) {}
};

// unique_ptr doesn't work with entityx, so we have to use shared
using piece_iface = std::shared_ptr<piece_interface>;

// information about a piece
struct piece_data
{
	piece_type type;
	int team;

	piece_data(piece_type init_type = piece_type::block, int init_team = 0)
		: type(init_type), team(init_team)
	{
	}
};
