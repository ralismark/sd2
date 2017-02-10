#include "core/runtime.cpp"
#include "include/randutils.hpp"

#include <sfml/graphics.hpp>

#include <algorithm>
#include <numeric>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <variant>

constexpr int data_size = 300;
constexpr int data_max = data_size;

template <typename First, typename Last>
void sort_algo(First first, Last last);

namespace central {

	std::thread sort_thread;
	std::atomic_bool sort_finish_request = false;
	std::atomic_bool sort_finished = false;

	namespace thread {

		// set when running
		// reset at end of render frame
		std::atomic_bool sort_run = false;
		std::atomic_bool render_run = false;

		// signal for the other thread to run
		std::condition_variable cv;

		struct element { // {{{
			// we are assuming that values in the list are never destructed, only swapped (or moved)
		public:
			template <typename F>
			static void locked_call(bool lockstep, F& fn);
			static bool is_temp(const element* e);
		public: // variables
			int value;
		public:
			element(int val)
				: value(val)
			{
			}

			element(element&& other)
				: value(other.value)
			{
				// no locked call needed
			}

			element& operator=(element&& other)
			{
				if(this == &other) {
					return *this;
				}

				auto fn = [&] {
					value = other.value;
				};

				bool is_important = !is_temp(this);

				locked_call(is_important, fn);

				return *this;
			}

			friend void swap(element& lhs, element& rhs)
			{
				auto fn = [&] {
					using std::swap;
					swap(lhs.value, rhs.value);
				};

				bool is_important = !is_temp(&lhs) || !is_temp(&rhs);

				locked_call(is_important, fn);
			}

			friend bool operator<(const element& lhs, const element& rhs)
			{
				bool out = false;

				auto fn = [&] {
					out = lhs.value < rhs.value;
				};

				bool is_important = false;//!is_temp(&lhs) || !is_temp(&rhs);

				locked_call(is_important, fn);
				return out;
			}
		};

		std::vector<element> data;
		std::mutex data_lock;

		template <typename F>
		void element::locked_call(bool lockstep, F& fn)
		{
			if(sort_finish_request) {
				rt::exit(1);
			}

			std::unique_lock<std::mutex> lock(data_lock);

			if(lockstep) {
				cv.wait(lock, [] { return sort_run.load(); });
				sort_run = false;
			}

			fn();

			if(lockstep) {
				render_run = true;
				lock.unlock();
				cv.notify_one();
			}
		}

		bool element::is_temp(const element* e)
		{
			std::lock_guard<std::mutex> lock(data_lock);
			bool in_data = &data.front() <= e && e < &data.back();

			return !in_data;
		}

		void data_init()
		{
			static randutils::mt19937_rng rng;

			// data array
			std::vector<int> values(data_size);
			std::iota(values.begin(), values.end(), 2);
			std::shuffle(values.begin(), values.end(), rng.engine());

			std::lock_guard<std::mutex> lock(data_lock);

			data.clear();
			data.reserve(data_size);

			for(int i = 0; i < data_size; ++i) {
				int value = i; // edit for various generators
				data.emplace_back(values[i]);
			}
		}

		void initial()
		{
			try {
				// run thread
				data_init();
				sort_algo(data.begin(), data.end());
			} catch(const std::exception& e) {
				std::clog << "uncaught exception: " << typeid(e).name() << '\n'
					<< "  .what(): " << e.what() << '\n';
				throw;
			} catch(const rt::detail::exit_signaller&) {
				// exit
			}
			sort_finished = true;
		}

	}

	int counter = 0; // for scroll-up effect at the end

	void render()
	{
		if(!sort_finished.load()) {
			{ // signal sorting thread to run
				thread::sort_run = true;
				thread::cv.notify_one();
			}

			{ // wait for worker
				std::unique_lock<std::mutex> lock(thread::data_lock);
				thread::cv.wait(lock, [] { return thread::render_run.load(); });
				thread::render_run = false;
			}
		}

		stdwin->clear(sf::Color::Black);

		sf::RectangleShape rect;
		rect.setOrigin(static_cast<float>(0), static_cast<float>(stdwin.winsize.y - 30));
		rect.setScale(1, -1);

		rect.setPosition(1, 0);
		double height_step = (stdwin.winsize.y - 60) / double(data_max);
		double width_step = stdwin.winsize.x / double(data_size);
		double width = width_step - 2;

		{
			std::lock_guard<std::mutex> lock(thread::data_lock);

			// no need to be atomic, we're locking the array
			bool sort_done = sort_finished.load();

			int scroll_step = data_size / stdwin.winfps;

			if(sort_done) {
				counter += scroll_step;
			}

			int i = counter;

			for(auto& elem : thread::data) {
				rect.setSize(sf::Vector2f(static_cast<float>(width), static_cast<float>(elem.value * height_step)));
				auto color = sf::Color::White;

				if(i > 0) {
					color = sf::Color::Green;
					--i;
				}

				rect.setFillColor(color);

				stdwin->draw(rect);
				rect.move(static_cast<float>(width_step), 0);
			}
		}

		stdwin->display();
	}

	void var_init()
	{
		sort_thread = std::thread(thread::initial);
	}

	void var_clean()
	{
		// make sort thread continue running
		thread::sort_run = true;
		thread::cv.notify_one();

		sort_finish_request = true;

		if(sort_thread.joinable()) {
			sort_thread.join();
		}
	}

} // namespace central

template <typename First, typename Last>
void sort_algo(First first, Last last)
{
	std::make_heap(first, last);
	std::sort_heap(first, last);
}

void initial()
{
	rt::on_frame.connect(central::render, 30);
	rt::on_cleanup.connect(central::var_clean, 0);

	central::var_init();
}
