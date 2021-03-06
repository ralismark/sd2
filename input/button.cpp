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

std::list<button::event> button::process(const sf::Event& e)
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
			return {};
		}

		vec2i pos(e.mouseMove.x, e.mouseMove.y);
		bool wincontained = winarea.contains(pos);
		bool contained = wincontained && this->contains(pos);

		if(condition == state::idle && contained) {
			return this->transition(state::hover);
		} else if(condition == state::hover && !contained) {
			return this->transition(state::idle);
		} else if(condition == state::active && !contained) {
			return this->transition(state::persist);
		} else if(condition == state::persist && contained) {
			return this->transition(state::active);
		}
	} else if(e.type == sf::Event::MouseButtonPressed) {
		/*
		 * o -> o
		 *      v
		 * o    O
		 */
		if(e.mouseButton.button != sf::Mouse::Left) {
			return {};
		}
		// a press requires focus - we don't care otherwise
		vec2i pos(e.mouseButton.x, e.mouseButton.y);
		bool wincontained = winarea.contains(pos);
		bool contained = wincontained && this->contains(pos);

		if(contained) {
			return this->transition(state::active);
		}
	} else if(e.type == sf::Event::MouseButtonReleased) {
		/*
		 * O    O
		 * ^    ^
		 * o <> o
		 */
		if(e.mouseButton.button != sf::Mouse::Left) {
			return {};
		}
		// note: this event may not trigger if focus is lost

		vec2i pos(e.mouseButton.x, e.mouseButton.y);
		bool wincontained = winarea.contains(pos);
		bool contained = wincontained && this->contains(pos);

		std::list<event> out;

		if(condition == state::persist && contained) {
			out = this->transition(state::active);
		} else if(condition == state::active && !contained) {
			out = this->transition(state::persist);
		}

		if(condition == state::persist) {
			auto steps = this->transition(state::idle);
			out.splice(out.end(), std::move(steps));
		} else if(condition == state::active) {
			auto steps = this->transition(state::hover);
			out.splice(out.end(), std::move(steps));
		}
		return out;
	} else if(e.type == sf::Event::LostFocus) {
		/*
		 * O <- o
		 * ^
		 * o <- o
		 */
		if(condition != state::idle) {
			return this->transition(state::idle);
		}
	} else if(e.type == sf::Event::MouseLeft) {
		/*
		 * O <- o
		 *
		 * O <- o
		 */
		if(condition == state::hover) {
			return this->transition(state::idle);
		} else if(condition == state::active) {
			return this->transition(state::persist);
		}
	}
	return {};
} // }}}

// }}}
