#pragma once

#include "afx.hpp"

// a set of sprites generated from a spritesheet (multiple sprites in one image)
// use dereference to access sf::Sprite
struct spriteset
{ // {{{
	sf::Image spritesheet;
	std::vector<sf::Texture> tex;
	std::size_t index = 0;
	double accumulate = 0;

	sf::Sprite sprite;

public:
	spriteset() = default;

	// load a set of sprites
	spriteset(const store::resource_type& res, std::size_t width, std::size_t height)
	{
		this->load(res, width, height);
	}

	// unload these sprites
	void unload()
	{
		tex.clear();
		index = 0;
		sprite = sf::Sprite{};
	}

	// load a block of data as a single sprite
	void load_whole(const void* data, std::size_t size)
	{
		this->unload();

		if(!spritesheet.loadFromMemory(static_cast<const uint8_t*>(data), size)) {
			throw std::runtime_error("image load error!");
		}
		auto area = spritesheet.getSize();

		sf::IntRect region = { 0, 0, area.x, area.y };
		sf::Texture current_tex;
		if(!current_tex.loadFromImage(spritesheet, region)) {
			throw std::runtime_error("individual sprite load error!");
		}
		tex.push_back(std::move(current_tex));

		sprite.setTexture(tex[0], true);
	}

	// load a spriteset normally
	void load(const void* data, std::size_t size, std::size_t width, std::size_t height)
	{
		this->unload();

		if(!spritesheet.loadFromMemory(static_cast<const uint8_t*>(data), size)) {
			throw std::runtime_error("image load error!");
		}
		auto area = spritesheet.getSize();

		for(std::size_t y = 0; y + width <= area.y; y += height) {
			for(std::size_t x = 0; x + height <= area.x; x += width) {
				sf::IntRect region = { x, y, width, height };
				sf::Texture sprite;
				if(!sprite.loadFromImage(spritesheet, region)) {
					throw std::runtime_error("individual sprite load error!");
				}
				tex.push_back(std::move(sprite));
			}
		}

		sprite.setTexture(tex[index], true);
	}

	// load a spriteset from a stored resource (wrapper around normal load)
	void load(const store::resource_type& res, std::size_t width, std::size_t height)
	{
		this->load(res.get(), res.size(), width, height);
	}

	// re-set the texture
	void refresh()
	{
		sprite.setTexture(tex[index], true);
	}

	// advance to the next sprite
	void next()
	{
		index = (index + 1) % tex.size();
		sprite.setTexture(tex[index], true);
	}

	// advance by a set amount
	// best for animations over time
	void advance(double dt)
	{
		accumulate += dt;
		while(accumulate > 1) {
			accumulate -= 1;
			this->next();
		}
	}

	sf::Sprite& operator*()
	{
		return sprite;
	}

	sf::Sprite* operator->()
	{
		return &sprite;
	}
}; // }}}

