#include "runtime.cpp"
#include "button.cpp"

namespace central {

	switchboard sb;

	void var_init()
	{
		button b(vec2i{0, 0}, vec2i{400, 400});
		sb.add(1, std::move(b));
	}

	void winevent(sf::Event e)
	{
		sb.process(e);
	}

}

void initial()
{
	rt::on_win_event.connect(central::winevent);

	central::var_init();
}
