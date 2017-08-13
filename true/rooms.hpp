#pragma once

#include "afx.hpp"

namespace rooms {

	void clear_blocks()
	{
		for(int x = 0; x < cfg::width; ++x) {
			for(int y = 0; y < cfg::height; ++y) {
				tile(x, y).blocked = false;
			}
		}
	}

	void apply_bitgrid(const bit_grid& blocks)
	{
		for(int i = 0; i < cfg::width * cfg::height; ++i) {
			tile(index_to_square(i)).blocked = blocks[i];
		}
	}

	// # for blocks
	// anything else for space
	bit_grid string_to_room(const char* s, unsigned int size)
	{
		bit_grid out;

		unsigned int i = 0;
		for(int y = 0; y < cfg::height; ++y) {
			int x = 0;
			while(i < size && s[i] != '\n') {
				if(x < cfg::width) {
					out[square_to_index({x, y})] = (s[i] == '#');
					++x;
				}
				++i;
			}
			++i;
		}

		return out;
	}

	// levels on lines after %
	std::map<char, std::vector<unsigned int>> load_offsets(const char* s, unsigned int size)
	{
		std::map<char, std::vector<unsigned int>> out;
		for(unsigned idx = 0; idx < size; ++idx) {
			if(s[idx] == '%') {
				++idx;
				char type = s[idx];
				while(idx < size && s[idx] != '\n') {
					++idx;
				}
				out[type].push_back(idx + 1);
			}
		}
		return out;
	}

	std::map<char, std::vector<unsigned int>>& get_offsets_cache(const store::resource_type& res)
	{
		static auto offsets = load_offsets((const char*)(res.get()), res.size());
		return offsets;
	}

	char get_category()
	{
		if(var::score < 300) {
			return 'a';
		}
		if(var::score < 700) {
			return rng.pick({ 'a', 'b' });
		}
		if(var::score < 750) {
			return '?'; // special level
		}
		if(var::score < 1000) {
			return 'b';
		}
		if(var::score < 1500) {
			return rng.pick({ 'b', 'c' });
		}
		return 'c';
	}

	void load_level(const store::resource_type& res, entityx::Entity player, entityx::Entity exit, int offset = -1)
	{
		clear_ai();
		remove_from_board(player);
		remove_from_board(exit);

		auto s = (const char*)(res.get());
		auto size = res.size();

		bit_grid out;

		if(offset < 0) {
			auto offsets = get_offsets_cache(res);
			offset = rng.pick(offsets[get_category()]);
		}

		unsigned int i = offset;

		for(int y = 0; y < cfg::height; ++y) {
			int x = 0;
			while(i < size && s[i] != '\n') {
				char c = s[i++];
				if(x >= cfg::width) {
					continue;
				}

				switch(c) {
				case '#':
					out[square_to_index({x, y})] = true;
					break;
				case '>':
					move_to(exit, {x, y});
					break;
				case '@':
					move_to(player, {x, y});
					break;
				case 'n':
					move_to(create_ai(store::knight_png, piece_type::knight), {x, y});
					break;
				case 'q':
					move_to(create_ai(store::queen_png, piece_type::queen), {x, y});
					break;
				case 'k':
					move_to(create_ai(store::king_png, piece_type::king), {x, y});
					break;
				case 'p':
					move_to(create_ai(store::pawn_png, piece_type::pawn), {x, y});
					break;
				case 'b':
					move_to(create_ai(store::bishop_png, piece_type::bishop), {x, y});
					break;
				case '?':
					move_to(create_prop(store::dragon_png), {x, y});
					break;
				}

				++x;
			}
			++i;
		}

		rooms::apply_bitgrid(out);
	}

} // namespace rooms
