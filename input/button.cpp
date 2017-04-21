#include "button.hpp"

#include "disp/window.hpp"

#include <cassert>

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
	using event_table = std::map<std::pair<state, state>, std::list<button::event>>;
	static const event_table transitions = {
		/*   state from      state to           triggered events */
		{ { state::idle,    state::idle    }, { /* none*/                                   } },
		{ { state::idle,    state::hover   }, { event::hover_on                             } },
		{ { state::idle,    state::active  }, { event::hover_on, event::press               } },
		{ { state::idle,    state::persist }, { event::hover_on, event::press, event::leave } },
		{ { state::hover,   state::idle    }, { event::hover_off                            } },
		{ { state::hover,   state::hover   }, { /* none*/                                   } },
		{ { state::hover,   state::active  }, { event::press                                } },
		{ { state::hover,   state::persist }, { event::press, event::leave                  } },
		{ { state::active,  state::idle    }, { event::leave, event::away_release           } },
		{ { state::active,  state::hover   }, { event::release                              } },
		{ { state::active,  state::active  }, { /* none*/                                   } },
		{ { state::active,  state::persist }, { event::leave                                } },
		{ { state::persist, state::idle    }, { event::away_release                         } },
		{ { state::persist, state::hover   }, { event::reenter, event::release              } },
		{ { state::persist, state::active  }, { event::reenter                              } },
		{ { state::persist, state::persist }, { /* none*/                                   } }
	};

	auto change_pair = std::make_pair(condition, new_state);
	auto events = transitions.find(change_pair);
	assert(events != transitions.end() && "Transition not valid, invalid enum values?");

	condition = new_state;
	return events->second;
}

void button::process(const sf::Event& e)
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
		bool wincontained = winarea.contains(pos);
		bool contained = wincontained && this->contains(pos);

		if(condition == state::idle && contained) {
			this->transition(state::hover);
		} else if(condition == state::hover && !contained) {
			this->transition(state::idle);
		} else if(condition == state::active && !contained) {
			this->transition(state::persist);
		} else if(condition == state::persist && contained) {
			this->transition(state::active);
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
		// a press requires focus - we don't care otherwise
		vec2i pos(e.mouseButton.x, e.mouseButton.y);
		bool wincontained = winarea.contains(pos);
		bool contained = wincontained && this->contains(pos);

		if(contained) {
			this->transition(pair.first, state::active);
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
		bool wincontained = winarea.contains(pos);
		bool contained = wincontained && this->contains(pos);

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
	} else if(e.type == sf::Event::LostFocus) {
		/*
		 * O <- o
		 * ^
		 * o <- o
		 */
		if(bi.condition != state::idle) {
			this->transition(pair.first, state::idle);
		}
	} else if(e.type == sf::Event::MouseLeft) {
		/*
		 * O <- o
		 *
		 * O <- o
		 */
		if(bi.condition == state::hover) {
			this->transition(pair.first, state::idle);
		} else if(bi.condition == state::active) {
			this->transition(pair.first, state::persist);
		}
	}
} // }}}

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
{
	for(auto&& pair : buttons) {
		auto& button = pair.second;
		button.process(e);
	}
}

// }}}
