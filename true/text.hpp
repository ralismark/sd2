#pragma once

#include "afx.hpp"

void draw_text(const char* str, vec2 pos, sf::Color color = sf::Color::White, unsigned int char_size = 30)
{
	static auto font = [] {
			sf::Font f;
			f.loadFromMemory(store::opensans_ttf.get(), store::opensans_ttf.size());
			return f;
		}();

	sf::Text text(str, font, char_size);
	text.setFillColor(color);
	text.setPosition(pos.x, pos.y);

	stdwin->draw(text);
}
