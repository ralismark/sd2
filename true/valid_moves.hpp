#pragma once

#include "afx.hpp"

#include "grid_iface.hpp"
#include "piece.hpp"

// check if a tile can be occupied by another piece
bool can_move_to(vec2u target)
{
	if(target.x >= cfg::width || target.y >= cfg::height) {
		// out of bounds
		return false;
	}

	if(tile(target).blocked) {
		// blocked (tile effectively doesn't exist)
		return false;
	}

	if(tile(target).piece && tile(target).piece.has_component<piece_data>()) {
		// already occupied by a piece
		return false;
	}

	return true;
}

// check if a tile's piece can be captured by a certain team
bool can_capture(vec2u target, int team)
{
	if(target.x >= cfg::width || target.y >= cfg::height) {
		// out of bounds
		return false;
	}

	auto tile_piece = tile(target).piece;
	if(!tile_piece) {
		// no occupant
		return false;
	}

	auto data = tile_piece.component<piece_data>();
	if(!data) {
		// no data
		return false;
	}

	// different teams
	return data->team != team;
}

template <unsigned int N>
bit_grid get_traced_moves(const std::array<vec2i, N>& directions, vec2u pos, int team)
{
	bit_grid out;

	// trace in each direction
	for(auto& dir : directions) {
		auto cursor = pos + dir;
		while(can_move_to(cursor) || can_capture(cursor, team)) {
			out[square_to_index(cursor)] = true;
			if(can_capture(cursor, team)) {
				break;
			}
			cursor += dir;
		}
	}

	return out;
}

template <unsigned int N>
bit_grid get_offset_moves(const std::array<vec2i, N>& offsets, vec2i pos, int team)
{
	bit_grid out;

	for(auto& offset : offsets) {
		auto target = pos + offset;
		if(can_move_to(target) || can_capture(target, team)) {
			out[square_to_index(target)] = true;
		}
	}

	return out;
}

bit_grid get_valid_rook_moves(vec2i pos, int team)
{
	static std::array<vec2i, 4> directions = {{
		{ 1, 0 },
		{ -1, 0 },
		{ 0, 1 },
		{ 0, -1 }
	}};

	return get_traced_moves(directions, pos, team);
}

bit_grid get_valid_bishop_moves(vec2u pos, int team)
{
	static std::array<vec2i, 4> directions = {{
		{ 1, 1 },
		{ -1, -1 },
		{ -1, 1 },
		{ 1, -1 }
	}};

	return get_traced_moves(directions, pos, team);
}

bit_grid get_valid_queen_moves(vec2u pos, int team)
{
	return get_valid_rook_moves(pos, team) | get_valid_bishop_moves(pos, team);
}

bit_grid get_valid_knight_moves(vec2u pos, int team)
{
	static std::array<vec2i, 8> offsets = {{
		{-2, 1},
		{-1, 2},
		{1, 2},
		{2, 1},
		{2, -1},
		{1, -2},
		{-1, -2},
		{-2, -1} }};

	return get_offset_moves(offsets, pos, team);
}

bit_grid get_valid_king_moves(vec2u pos, int team)
{
	static std::array<vec2i, 8> offsets = {{
		{ -1, -1 },
		{ -1, 0 },
		{ -1, 1 },
		{ 0, -1 },
		// { 0, 0 },
		{ 0, 1 },
		{ 1, -1 },
		{ 1, 0 },
		{ 1, 1 },
	}};

	return get_offset_moves(offsets, pos, team);
}

bit_grid get_valid_pawn_moves(vec2u pos, int team)
{
	static std::array<vec2i, 8> offsets = {{
		{ -1, 0 },
		{ 0, -1 },
		{ 0, 1 },
		{ 1, 0 },
	}};

	return get_offset_moves(offsets, pos, team);
}

bit_grid get_valid_moves(vec2u pos, int team, piece_type pt)
{
	bit_grid out;
	switch(pt) {
	case piece_type::knight:
		out = get_valid_knight_moves(pos, team);
		break;
	case piece_type::bishop:
		out =  get_valid_bishop_moves(pos, team);
		break;
	case piece_type::rook:
		out =  get_valid_rook_moves(pos, team);
		break;
	case piece_type::queen:
		out =  get_valid_queen_moves(pos, team);
		break;
	case piece_type::king:
		out =  get_valid_king_moves(pos, team);
		break;
	case piece_type::pawn:
		out =  get_valid_pawn_moves(pos, team);
		break;
	default:
		break;
	}
	out[square_to_index(pos)] = true;
	return out;
}

bit_grid& ensure_bitgrid(entityx::Entity e)
{
	auto moves = e.component<bit_grid>();
	if(!moves) {
		auto pos = e.component<vec2u>();
		auto data = e.component<piece_data>();

		if(pos && data) {
			e.assign<bit_grid>(get_valid_moves(*pos, data->team, data->type));
		} else {
			e.assign<bit_grid>();
		}
		return *e.component<bit_grid>();
	} else {
		return *moves;
	}
}
