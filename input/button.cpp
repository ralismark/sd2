#include "button.hpp"

#include "disp/window.hpp"

#include <cassert>

static event get_event(state from, state to)
{
	if(from == state::idle && to == state::hover) {
		return event::hover_on;
	}
	if(from == state::hover && to == state::idle) {
		return event::hover_off;
	}

	if(from == state::hover && to == state::active) {
		return event::press;
	}
	if(from == state::active && to == state::hover) {
		return event::release;
	}

	if(from == state::active && to == state::persist) {
		return event::leave;
	}
	if(from == state::persist && to == state::active) {
		return event::reenter;
	}

	if(from == state::persist && to == state::idle) {
		return event::away_release;
	}

	assert(false && "invalid state transition");
	return {};
}

// class button {{{

button::button()
	: bound()
	, condition(state::idle)
{
}

button::button(const vector_type& pos, const vector_type& size)
	: bound(pos.x, pos.y, size.x, size.y)
	, condition(state::idle)
{
}

button::button(const sf::Rect<dimension_type>& init_bound)
	: bound(init_bound)
	, condition(state::idle)
{
}

bool button::contains(const vector_type& pos) const
{
	return bound.contains(pos.x, pos.y);
}

button::state button::current_state() const
{
	return condition;
}

sf::Rect<button::dimension_type> button::region() const
{
	return bound;
}

std::list<button::event> button::region(sf::Rect<dimension_type> new_bound)
{
	bound = new_bound;
	return this->transition(state::idle);
}

std::list<button::event> button::transition(state new_state)
{
	std::list<event> out;

	if(new_state == condition) {
		return out; // null transition
	}

	// ensure only valid transitions happen
	// this is done through defined intermediary steps
	// then the main transition takes place

	if(condition == state::idle && new_state == state::active) {
		auto step = this->transition(id, state::hover);

		out.splice(out.end(), std::move(step));
	} else if(condition == state::idle && new_state == state::persist) {
		auto step1 = this->transition(id, state::hover);
		auto step2 = this->transition(id, state::active);

		out.splice(out.end(), std::move(step1));
		out.splice(out.end(), std::move(step2));
	} else if(condition == state::hover && new_state == state::persist) {
		auto step = this->transition(id, state::active);

		out.splice(out.end(), std::move(step));
	} else if(condition == state::active && new_state == state::idle) {
		auto step = this->transition(id, state::persist);

		out.splice(out.end(), std::move(step));
	} else if(condition == state::persist && new_state == state::hover) {
		auto step = this->transition(id, state::active);

		out.splice(out.end(), std::move(step));
	}

	out.emplace_back(get_event(condition, new_state));
	condition = new_state;

	return out;
}

// }}}

// class switchboard {{{

void switchboard::transition(id_type id, state new_state)
{
	assert(buttons.count(id) > 0 && "button must exist");

	buttons[id].transition(new_state);
}

bool switchboard::exists(id_type id) const
{
	return buttons.count(id) > 0;
}

bool switchboard::add(id_type id, button b)
{
	if(this->exists(id)) {
		// id exists
		buttons[id] = std::move(b);
		return false;
	} else {
		// new id
		buttons.emplace(id, std::move(b));
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
		return &it->second;
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

			bool contained = wincontained && bi.b.contains(pos);
			if(bi.condition == state::idle && contained) {
				this->transition(pair.first, state::hover);
			} else if(bi.condition == state::hover && !contained) {
				this->transition(pair.first, state::idle);
			} else if(bi.condition == state::active && !contained) {
				this->transition(pair.first, state::persist);
			} else if(bi.condition == state::persist && contained) {
				this->transition(pair.first, state::active);
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

			bool contained = wincontained && bi.b.contains(pos);
			if(contained) {
				this->transition(pair.first, state::active);
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

			bool contained = wincontained && bi.b.contains(pos);
			if(bi.condition == state::persist && contained) {
				this->transition(pair.first, state::active);
			} else if(bi.condition == state::active && !contained) {
				this->transition(pair.first, state::persist);
			}

			if(bi.condition == state::persist) {
				this->transition(pair.first, state::idle);
			} else if(bi.condition == state::active) {
				this->transition(pair.first, state::hover);
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

			if(bi.condition != state::idle) {
				this->transition(pair.first, state::idle);
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

			if(bi.condition == state::hover) {
				this->transition(pair.first, state::idle);
			}
			if(bi.condition == state::active) {
				this->transition(pair.first, state::persist);
			}
		}
	}
} // }}}

// }}}
