#include "button.hpp"

#include "disp/window.hpp"

// class button {{{

button::button()
	: bound()
	, active(false)
{
}

button::button(const vector_type& pos, const vector_type& size)
	: bound(pos.x, pos.y, size.x, size.y)
	, active(false)
{
}

button::button(const sf::Rect<dimension_type>& init_bound)
	: bound(init_bound)
	, active(false)
{
}

bool button::contains(const vector_type& pos) const
{
	return bound.contains(pos.x, pos.y);
}

// }}}

// class switchboard {{{

// struct binfo {{{

void switchboard::binfo::transition(state new_s)
{
	if(new_s == s) {
		return; // not a transition
	}

	// ensure only valid transitions happen
	// this is done through defined intermediary steps
	// then the main transition takes place

	/*  */ if(s == state::idle && new_s == state::active) {
		this->transition(state::hover);
	} else if(s == state::idle && new_s == state::persist) {
		this->transition(state::hover);
		this->transition(state::active);
	} else if(s == state::hover && new_s == state::persist) {
		this->transition(state::active);
	} else if(s == state::active && new_s == state::idle) {
		this->transition(state::persist);
	} else if(s == state::persist && new_s == state::hover) {
		this->transition(state::active);
	}

	s = new_s;
}

// }}}

bool switchboard::exists(id_type id) const
{
	return buttons.count(id) > 0;
}

bool switchboard::add(id_type id, button b)
{
	if(this->exists(id)) {
		// id exists
		buttons[id] = binfo{std::move(b), state::idle};
		return false;
	} else {
		// new id
		buttons.emplace(id, binfo{std::move(b), state::idle});
		return true;
	}
}

void switchboard::remove(id_type id)
{
	buttons.erase(id);
}

const button* switchboard::get(id_type id) const
{
	auto it = buttons.find(id);
	if(it != buttons.end()) {
		return &it->second.b;
	} else {
		return nullptr;
	}
}

button* switchboard::get(id_type id)
{
	// non-const based on const impl
	const switchboard* cthis = this;
	auto const_out = cthis->get(id);
	return const_cast<button*>(const_out);
}

// also false if not present
bool switchboard::is_active(id_type id) const
{
	if(!this->exists(id)) {
		return false;
	}
	return this->get(id)->active;
}

void switchboard::set_state(id_type id, bool active)
{
	if(this->exists(id)) {
		buttons[id].b.active = active;
		buttons[id].transition(state::idle); // either mode must begin from idle
	}
}

void switchboard::process(const sf::Event& e)
{ // {{{
	/*
	 * possible transition are displayed in a diagram
	 *
	 * top left:     idle
	 * top right:    hover
	 * bottom left:  persist
	 * bottom right: active
	 *
	 * small o's are states
	 * large O's are terminating states
	 */
	static auto winarea = sf::Rect<button::dimension_type>(0, 0, stdwin.winsize.x, stdwin.winsize.y);

	if(e.type == sf::Event::MouseMoved) {
		/*
		 * O <> O
		 *
		 * O <> O
		 */
		// only trigger if has focus
		if(!stdwin->hasFocus()) {
			return;
		}

		vec2i pos(e.mouseMove.x, e.mouseMove.y);
		bool wincontained = winarea.contains(pos.x, pos.y);

		for(auto& pair : buttons) {
			auto& bi = pair.second;
			if(!bi.b.active) {
				continue;
			}

			bool contained = wincontained && bi.b.contains(pos);
			if(bi.s == state::idle && contained) {
				bi.transition(state::hover);
			} else if(bi.s == state::hover && !contained) {
				bi.transition(state::idle);
			} else if(bi.s == state::active && !contained) {
				bi.transition(state::persist);
			} else if(bi.s == state::persist && contained) {
				bi.transition(state::active);
			}
		}
	} else if(e.type == sf::Event::MouseButtonPressed) {
		/*
		 * o -> o
		 *      v
		 * o    O
		 */
		if(e.mouseButton.button != sf::Mouse::Left) {
			return;
		}
		// a press requires focus - we don't care
		vec2i pos(e.mouseButton.x, e.mouseButton.y);
		bool wincontained = winarea.contains(pos.x, pos.y);

		for(auto& pair : buttons) {
			auto& bi = pair.second;
			if(!bi.b.active) {
				continue;
			}

			bool contained = wincontained && bi.b.contains(pos);
			if(contained) {
				bi.transition(state::active);
			}
		}
	} else if(e.type == sf::Event::MouseButtonReleased) {
		/*
		 * O    O
		 * ^    ^
		 * o <> o
		 */
		if(e.mouseButton.button != sf::Mouse::Left) {
			return;
		}
		// note: this event may not trigger if focus is lost

		vec2i pos(e.mouseButton.x, e.mouseButton.y);
		bool wincontained = winarea.contains(pos.x, pos.y);

		for(auto& pair : buttons) {
			auto& bi = pair.second;
			if(!bi.b.active) {
				continue;
			}

			bool contained = wincontained && bi.b.contains(pos);
			if(bi.s == state::persist && contained) {
				bi.transition(state::active);
			} else if(bi.s == state::active && !contained) {
				bi.transition(state::persist);
			}

			if(bi.s == state::persist) {
				bi.transition(state::idle);
			} else if(bi.s == state::active) {
				bi.transition(state::hover);
			}
		}
	} else if(e.type == sf::Event::LostFocus) {
		/*
		 * O <- o
		 * ^
		 * o <- o
		 */
		for(auto& pair : buttons) {
			auto& bi = pair.second;
			if(!bi.b.active) {
				continue;
			}

			if(bi.s != state::idle) {
				bi.transition(state::idle);
			}
		}
	} else if(e.type == sf::Event::MouseLeft) {
		/*
		 * O <- o
		 *
		 * O <- o
		 */
		for(auto& pair : buttons) {
			auto& bi = pair.second;
			if(!bi.b.active) {
				continue;
			}

			if(bi.s == state::hover) {
				bi.transition(state::idle);
			}
			if(bi.s == state::active) {
				bi.transition(state::persist);
			}
		}
	}
} // }}}

// }}}
