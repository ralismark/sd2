#pragma once

#include <map>

#include <sfml/Graphics/Rect.hpp>

#include "include/sigslots.hpp"
#include "include/vector.hpp"

class button
{
public: // statics
	using dimension_type = int;
	using vector_type = vector<int, 2>;
public: // variables
	sf::Rect<dimension_type> bound;
	bool active;
public: // methods
	button()
		: bound()
		, active(false)
	{
	}

	button(const vector_type& pos, const vector_type& size)
		: bound(pos.x, pos.y, size.x, size.y)
		, active(false)
	{
	}

	button(const sf::Rect<dimension_type>& init_bound)
		: bound(init_bound)
		, active(false)
	{
	}

	bool contains(const vector_type& pos)
	{
		return bound.contains(pos.x, pos.y);
	}
};

class switchboard // group of buttons
{
	friend class sboard_ctl;
public: // statics
	using id_type = int;

	/*
	 * The state of a button and the cursor can be represented as a state
	 * machine, with 4 distinct states:
	 */
	enum class state {
		idle,    // not pressed, cursor away
		hover,   // not pressed, cursor over
		active,   // is  pressed, cursor over
		persist, // is  pressed, cursor away
	};
	/*
	 * only one status can change in a state transition - either the
	 * proximity (away or over) and the press (or not) of the button.
	 * Between these states, there are 8 possible transitions
	 */
	enum class event {
		hover_on, // idle -> hover
		hover_off, // hover -> idle
		press, // hover -> active
		release, // press -> active
		reenter, // persist -> active
		leave, // active -> persist
		away_release, // persist -> idle
		// cannot happen: away_press, // idle -> persist
	};

	/*
	 * Since the two variables (press and proximity) are controlled by separate events:
	 * MouseMoved for proximity and MouseButtonPressed and
	 * MouseButtonReleased, there is no way for ambiguity between
	 * transitions, e.g. the mouse both moves and presses the button.
	 */

	// TODO(timmy): now to implement it!

private: // internal statics

	struct binfo {
		button b;
		state s;

		void transition(state new_s)
		{
			// ensure only valid transitions happen
			// this is done through defined intermediary steps
			bool istep = true;

			/*  */ if(s == state::idle && new_s == state::active) {
				this->transition(state::hover);
				this->transition(state::active);
			} else if(s == state::idle && new_s == state::persist) {
				this->transition(state::hover);
				this->transition(state::active);
				this->transition(state::persist);
			} else if(s == state::hover && new_s == state::persist) {
				this->transition(state::active);
				this->transition(state::persist);
			} else if(s == state::active && new_s == state::idle) {
				this->transition(state::persist);
				this->transition(state::idle);
			} else if(s == state::persist && new_s == state::hover) {
				this->transition(state::active);
				this->transition(state::hover);
			} else {
				istep = false;
			}

			switch(s) {
			case state::idle:
				std::cout << "idle";
				break;
			case state::hover:
				std::cout << "hover";
				break;
			case state::active:
				std::cout << "active";
				break;
			case state::persist:
				std::cout << "persist";
				break;
			}

			std::cout << " -> ";

			switch(new_s) {
			case state::idle:
				std::cout << "idle";
				break;
			case state::hover:
				std::cout << "hover";
				break;
			case state::active:
				std::cout << "active";
				break;
			case state::persist:
				std::cout << "persist";
				break;
			}
			std::cout << '\n';

			s = new_s;
		}
	};

private: // variables

	std::map<id_type, binfo> buttons;
	vec2i prev;

public: // variables
public: // statics

	// {{{ interface

	bool exists(id_type id) const
	{
		return buttons.count(id) > 0;
	}

	bool add(id_type id, button b)
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

	void remove(id_type id)
	{
		buttons.erase(id);
	}

	const button* get(id_type id) const
	{
		auto it = buttons.find(id);
		if(it != buttons.end()) {
			return &it->second.b;
		} else {
			return nullptr;
		}
	}

	button* get(id_type id)
	{
		// non-const based on const impl
		const switchboard* cthis = this;
		auto const_out = cthis->get(id);
		return const_cast<button*>(const_out);
	}

	// also false if not present
	bool is_active(id_type id) const
	{
		if(!this->exists(id)) {
			return false;
		}
		return this->get(id)->active;
	}

	void set_state(id_type id, bool active)
	{
		if(this->exists(id)) {
			this->get(id)->active = active;
		}
	}

	// }}}

	void process(const sf::Event& e)
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

			for(auto& pair : buttons) {
				auto& bi = pair.second;

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
			bool wincontained = winarea.contains(pos);

			for(auto& pair : buttons) {
				auto& bi = pair.second;

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
			bool wincontained = winarea.contains(pos);

			for(auto& pair : buttons) {
				auto& bi = pair.second;

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

				if(bi.s == state::hover) {
					bi.transition(state::idle);
				}
				if(bi.s == state::active) {
					bi.transition(state::persist);
				}
			}
		}
	} // }}}

};
