#pragma once

#include "afx.hpp"

// AI implemented using a weighted random
// Each tile movable to by the enemy is given a score based on several factors:
// 1. can capture the player
// 2. avoids capture by the player
// 3. covers squared movable to by the player
// 4. proximity to the player
//
// A weighted random ensures that the AI is not perfect, and that the player
// will be able to capture pieces

// vec2 sometimees crashes during comparison. Don't know why. We're using pair as a workaround
std::map<std::pair<unsigned int, unsigned int>, int> get_weights(entityx::Entity e, entityx::Entity target)
{
	if(!e.component<vec2u>()) {
		return {};
	}

	std::vector<vec2u> moves;
	auto& valid_moves = ensure_bitgrid(e);
	for(int i = 0; i < cfg::width * cfg::height; ++i) {
		if(valid_moves[i]) {
			moves.push_back(index_to_square(i));
		}
	}

	auto& pdata = *e.component<piece_data>();

	std::map<std::pair<unsigned int, unsigned int>, int> scores;

	auto tpos = *target.component<vec2u>();
	auto& tmoves = ensure_bitgrid(target);

	std::sort(moves.begin(), moves.end(), [&] (const vec2u& a, const vec2u& b) {
		return velm::distance(a, tpos) < velm::distance(b, tpos);
		});

	int counter = moves.size();
	for(auto& move : moves) {
		int score = 0;

		if(move.x == tpos.x && move.y == tpos.y) {
			// basically always capture if possible
			score += 100;
		}

		if(!tmoves[square_to_index(move)]) {
			// reward for avoiding capture
			score += 3;
		}

		auto next_moves = get_valid_moves(move, pdata.team, pdata.type);
		score += 5 * (next_moves & tmoves).count(); // +5 for each square covered
		if(next_moves[square_to_index(tpos)]) {
			score += 2; // +2 for threatening piece
		}

		// reward for proximity (by rank)
		score += counter;
		--counter;

		if(move.x == e.component<vec2u>()->x && move.y == e.component<vec2u>()->y) {
			score /= 2;
		}

		// adjusted for better weighing
		// better squares are much more likely
		int adjusted_score = score * score;

		scores[std::make_pair(move.x, move.y)] = adjusted_score;
	}

	return scores;
}

// this is used for multiple players, if implemented
std::map<std::pair<unsigned int, unsigned int>, int> get_overall_weights(entityx::Entity e)
{
	std::map<std::pair<unsigned int, unsigned int>, int> scores;
	var::world.entities.each<vec2u, tag::ai_target>([&] (entityx::Entity target, auto, auto) {
		auto target_scores = get_weights(e, target);
		for(auto& move : target_scores) {
			scores[move.first] += move.second;
		}
	});
	return scores;
}

rt::delay_runner move_transition(entityx::Entity e, vec2u target, rt::delay_runner run = {});

void move_all_ai(rt::delay_runner run = {})
{
	var::world.entities.each<vec2u, tag::ai_control>([=] (entityx::Entity ai, auto, auto) {
		auto scores = get_overall_weights(ai);

		// decompose the map
		std::vector<vec2u> moves;
		std::vector<int> values;
		for(auto& entry : scores) {
			moves.emplace_back(entry.first.first, entry.first.second);
			values.emplace_back(entry.second);
		}

		if(values.size() == 0) {
			return; // cannot move
		}

		auto idx = rng.variate<int, std::discrete_distribution>(values.begin(), values.end());
		move_transition(ai, moves[idx], run);
	});
}
