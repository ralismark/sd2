#include "core/runtime.cpp"
#include "include/randutils.hpp"
#include "res0.hpp"

#include <SFML/Graphics.hpp>

#include <future>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>

#include <algorithm>
#include <numeric>
#include <variant>

constexpr int data_size = 300;
constexpr int data_max = data_size;

template <typename Iter>
void sort_algo(Iter first, Iter last);

enum class method : int {
	bubblesort,
	std_sort,
	std_stable_sort,
	heapsort,
	quicksort,
	mergesort,
	insertionsort,
	bitonicsort,
	halfsort,
	merge2sort,
	permute,
	shuffle,
};

std::string get_name(method m)
{
	switch(m) {
	case method::bubblesort:
		return "bubble sort";
	case method::std_sort:
		return "std::sort";
	case method::std_stable_sort:
		return "std::stable_sort";
	case method::heapsort:
		return "heapsort";
	case method::quicksort:
		return "quicksort";
	case method::mergesort:
		return "mergesort";
	case method::insertionsort:
		return "insertion sort";
	case method::bitonicsort:
		return "bitonic sort";
	case method::halfsort:
		return "half sort";
	case method::merge2sort:
		return "merge (variant)";
	case method::permute:
		return "permutations";
	case method::shuffle:
		return "shuffles";
	}
	return "unknown";
}

int parallel_limit()
{
	return 99;
	auto lim = std::thread::hardware_concurrency() - 1;
	if(lim == 0) {
		return (1 << 3) - 1;
	}
	return lim;
}

std::atomic<int> parallel_counter = 0;

template <typename F1, typename F2>
void run_parallel(F1&& f1, F2&& f2)
{
	if(parallel_counter < parallel_limit()) {
		++parallel_counter;
		auto task = std::async([f1] {
				f1();
				--parallel_counter;
			});
		f2();
	} else {
		f1();
		f2();
	}
}

method get_current_method();

namespace central {

	static randutils::mt19937_rng rng;

	std::thread sort_thread;
	std::atomic<bool> sort_finish_request = false;
	std::atomic<bool> sort_finished = false;
	std::atomic<bool> shuffling = false;

	std::atomic<int> assign_count = 0;
	std::atomic<int> compare_count = 0;

	void reset_counters()
	{
		assign_count = 0;
		compare_count = 0;
	}

	sf::Font proggy_clean;
	sf::Text header;

	namespace thread { // {{{

		// set when running
		// reset at end of render frame
		std::atomic<bool> sort_run = false;
		std::atomic<bool> render_run = false;

		// signal for the other thread to run
		std::condition_variable sort_cv;
		std::condition_variable main_cv;

		struct element { // {{{
			// we are assuming that values in the list are never destructed, only swapped (or moved)
		public:
			template <typename F>
			static void locked_call(bool lockstep, F& fn);
			static bool is_temp(const element* e);
		public: // variables
			int value;
			mutable int hi_mode;
		public:
			element(int val)
				: value(val), hi_mode(0)
			{
			}

			element(element&& other)
				: value(other.value), hi_mode(0)
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
					this->hi_mode = 1;
					other.hi_mode = 1;

					++assign_count;
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
					lhs.hi_mode = 1;
					rhs.hi_mode = 1;

					++assign_count;
				};

				bool is_important = !is_temp(&lhs) || !is_temp(&rhs);

				locked_call(is_important, fn);
			}

			friend bool operator<(const element& lhs, const element& rhs)
			{
				bool out = false;

				auto fn = [&] {
					out = lhs.value < rhs.value;
					lhs.hi_mode = 2;
					rhs.hi_mode = 2;

					++compare_count;
				};

				bool is_important = false; // !is_temp(&lhs) || !is_temp(&rhs);

				locked_call(is_important, fn);
				return out;
			}

			friend bool operator>(const element& lhs, const element& rhs)
			{
				return rhs < lhs;
			}
			friend bool operator==(const element& lhs, const element& rhs)
			{
				return !(lhs < rhs) && !(rhs < lhs);
			}
			friend bool operator!=(const element& lhs, const element& rhs)
			{
				return lhs < rhs || rhs < lhs;
			}
			friend bool operator<=(const element& lhs, const element& rhs)
			{
				return !(rhs < lhs);
			}
			friend bool operator>=(const element& lhs, const element& rhs)
			{
				return !(lhs < rhs);
			}

		}; // }}}

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
				sort_cv.wait(lock, [] { return sort_run.load(); });
				sort_run = false;
			}

			fn();

			if(lockstep) {
				render_run = true;
				lock.unlock();
				main_cv.notify_one();
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
			// data array
			std::vector<int> values(data_size);
			std::iota(values.begin(), values.end(), 2);
			std::shuffle(values.begin(), values.end(), rng.engine());

			std::lock_guard<std::mutex> lock(data_lock);

			data.clear();
			data.reserve(data_size);

			for(int i = 0; i < data_size; ++i) {
				data.emplace_back(values[i]);
			}
		}

		void initial()
		{
			try {
				// run thread
				data_init();
				bool direction = false;
				while(1) {
					sort_algo(data.begin(), data.end());
					reset_counters();
					shuffling = true;

					if(direction) {
						std::shuffle(data.begin(), data.end(), rng.engine());
					} else {
						std::shuffle(data.rbegin(), data.rend(), rng.engine());
					}
					direction = !direction;

					reset_counters();
					shuffling = false;
				}
			} catch(const std::exception& e) {
				std::clog << "uncaught exception: " << typeid(e).name() << '\n'
					<< "  .what(): " << e.what() << '\n';
				throw;
			} catch(const rt::detail::exit_signaller&) {
				// exit
			}
			sort_finished = true;
		}

	} // }}}

	double counter = 0; // for scroll-up effect at the end

	void render()
	{
		if(!sort_finished.load()) {
			{ // signal sorting thread to run
				thread::sort_run = true;
				thread::sort_cv.notify_one();
			}

			{ // wait for worker
				std::unique_lock<std::mutex> lock(thread::data_lock);
				thread::main_cv.wait(lock, [] { return thread::render_run.load(); });
				thread::render_run = false;
			}
		}

		stdwin->clear(sf::Color::Black);

		sf::RectangleShape rect;
		rect.setOrigin(static_cast<float>(0), static_cast<float>(stdwin.winsize.y));
		rect.setScale(1, -1);

		rect.setPosition(0, 0);
		double height_step = (stdwin.winsize.y - 30) / double(data_max);
		double width_step = stdwin.winsize.x / double(data_size);
		double width = width_step - 0;
		// rect.setSize(sf::Vector2f(std::max(width, 1.0), 1));

		{
			std::lock_guard<std::mutex> lock(thread::data_lock);

			// no need to be atomic, we're locking the array
			bool sort_done = sort_finished.load();

			double scroll_step = double(data_size) / stdwin.winfps;
			if(sort_done) {
				counter += scroll_step;
			}

			int i = static_cast<int>(counter);

			for(auto& elem : thread::data) {
				rect.setSize(sf::Vector2f(static_cast<float>(width), static_cast<float>(elem.value * height_step)));
				// rect.setPosition(rect.getPosition().x, -elem.value * height_step);
				auto color = sf::Color::White;

				if(elem.hi_mode) {
					color = elem.hi_mode == 1 ? sf::Color::Red : sf::Color::Cyan;
					elem.hi_mode = 0;
				} else if(i > 0) {
					color = sf::Color::Green;
					--i;
				}

				rect.setFillColor(color);

				stdwin->draw(rect);
				rect.move(static_cast<float>(width_step), 0);
			}
		}

		std::string head = (shuffling ? "shuffling..." : get_name(get_current_method())) + " - " + std::to_string(assign_count) + " assignments, " + std::to_string(compare_count) + " comparisons";

		header.setString(head.c_str());
		stdwin->draw(header);

		stdwin->display();
	}

	void var_init()
	{
		if(!proggy_clean.loadFromMemory(store::monofonto_ttf.get(), store::monofonto_ttf.size())) {
			rt::exit(1);
		}
		header.setFont(proggy_clean);
		header.setFillColor(sf::Color::White);
		header.setCharacterSize(16);

		sort_thread = std::thread(thread::initial);
	}

	void var_clean()
	{
		sort_finish_request = true;

		// make sort threads continue running
		while(!sort_finished && sort_thread.joinable()) {
			thread::sort_run = true;
			thread::sort_cv.notify_one();
		}

		if(sort_thread.joinable()) {
			sort_thread.join();
		}
	}

} // namespace central

namespace salgo { // {{{

	template <typename Iter>
	void bitonicmerge(Iter first, Iter last, bool dir)
	{
		auto dist = last - first;
		if(dist <= 1) {
			return;
		}

		decltype(dist) m = 1;

		// largest multiple of 2 under dist
		while(m < dist) {
			m <<= 1;
		}
		m >>= 1;

		Iter mid = first + m;

		for(auto a = first, b = mid; b != last; ++a, ++b) {
			if((*b < *a) == dir) {
				std::iter_swap(a, b);
			}
		}

		run_parallel([&] {
				bitonicmerge(first, mid, dir);
			}, [&] {
				bitonicmerge(mid, last, dir);
			});
	}

	template <typename Iter>
	void bitonicsort(Iter first, Iter last, bool dir = true)
	{
		auto dist = last - first;
		if(dist <= 1) {
			return;
		}
		auto mid = first + (dist / 2);

		run_parallel([&] {
				bitonicsort(first, mid, !dir);
			}, [&] {
				bitonicsort(mid, last, dir);
			});

		bitonicmerge(first, last, dir);
	}

	template<class Iter>
	void mergesort(Iter first, Iter last)
	{
		auto dist = last - first;
		if(dist <= 1) {
			return;
		}
		Iter middle = first + dist / 2;

		run_parallel([&] {
				mergesort(first, middle);
			}, [&] {
				mergesort(middle, last);
			});

		std::inplace_merge(first, middle, last);
	}

	template <typename Iter>
	void quicksort(Iter first, Iter last)
	{
		if(first == last) {
			return;
		}

		auto pivot_point = last - 1;
		auto& pivot = *pivot_point;
		auto partpoint = std::partition(first, pivot_point, [&](const auto& e) { return e <= pivot; });
		std::iter_swap(pivot_point, partpoint);

		run_parallel([&] {
				quicksort(first, partpoint);
			}, [&] {
				quicksort(partpoint + 1, last);
			});
	}

	template <typename Iter>
	void bubblesort(Iter first, Iter last)
	{
		bool swapped = true;
		while(swapped) {
			swapped = false;

			for(auto front = first, back = front++;
			    front != last;
			    ++front, ++back) {
				if(*front < *back) {
					std::iter_swap(front, back);
					swapped = true;
				}
			}

			--last;
		}
	}

	template <typename Iter>
	void insertionsort(Iter first, Iter last)
	{
		for(auto i = first; i != last; ++i) {
			std::rotate(std::upper_bound(first, i, *i), i, i + 1);
		}
	}

	template <typename Iter>
	void halfsort(Iter first, Iter last, int depth = 0)
	{
		auto dist = last - first;
		if(dist > 1) {
			auto mid = first + dist / 2;
			std::nth_element(first, mid, last);

			if((1 << depth) < parallel_limit()) {
				auto task = std::async(std::launch::async, halfsort<Iter>, first, mid, depth + 1);
				halfsort(mid, last, depth + 1);
				task.wait();
			} else {
				halfsort(first, mid, depth + 1);
				halfsort(mid, last, depth + 1);
			}
		}
	}

	template <typename Iter>
	void merge2sort(Iter first, Iter last)
	{
		int step = 1;
		while(step < last - first) {
			auto left = first, mid = first + step;
			while(mid < last) {
				auto right = (last - mid > step) ? mid + step : last;

				std::inplace_merge(left, mid, right);

				left = right;
				if(last - right > step) {
					mid = right + step;
				} else {
					break;
				}
			}

			step <<= 1;
		}
	}

} // }}}

static constexpr method selected_sorts[] = {
	// method::bubblesort,
	method::bitonicsort,
	method::quicksort,
	method::mergesort,
	method::merge2sort,
	method::std_sort,
	method::std_stable_sort,
	method::heapsort,
	// method::halfsort,
	method::insertionsort,
	// method::permute,
	// method::shuffle,
	};
int algo_method = 0;

method get_current_method()
{
	return selected_sorts[algo_method];
}

template <typename Iter>
void sort_algo(Iter first, Iter last)
{
	switch(get_current_method()) {
	case method::bubblesort:
		salgo::bubblesort(first, last);
		break;
	case method::std_sort:
		std::sort(first, last);
		break;
	case method::std_stable_sort:
		std::stable_sort(first, last);
		break;
	case method::heapsort:
		std::make_heap(first, last);
		std::sort_heap(first, last);
		break;
	case method::quicksort:
		salgo::quicksort(first, last);
		break;
	case method::mergesort:
		salgo::mergesort(first, last);
		break;
	case method::insertionsort:
		salgo::insertionsort(first, last);
		break;
	case method::bitonicsort:
		salgo::bitonicsort(first, last);
		break;
	case method::halfsort:
		salgo::halfsort(first, last);
		break;
	case method::merge2sort:
		salgo::merge2sort(first, last);
		break;
	case method::permute:
		std::sort(first, last);
		while(1) {
			std::next_permutation(first, last);
		}
		break;
	case method::shuffle:
		while(1) {
			std::shuffle(first, last, central::rng.engine());
		}
		break;
	}

	algo_method = (algo_method + 1) % (sizeof(selected_sorts) / sizeof(selected_sorts[0]));
}

void initial()
{
	rt::on_frame.connect(central::render, 30);
	rt::on_cleanup.connect(central::var_clean, 0);

	central::var_init();
}
