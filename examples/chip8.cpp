#include "core/runtime.cpp"

#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <fstream>
#include <initializer_list>
#include <iomanip>
#include <iostream>

#include <cassert>
#include <cstdint>

#include <chrono_io>
#include <sfml/Graphics/RectangleShape.hpp>

#include "include/vector.hpp"
#include "include/randutils.hpp"
#include "include/fmt.hpp"
#include "input/keystate.hpp"

std::array<keystate, 16> keybinds { {
	sf::Keyboard::X,
	sf::Keyboard::Num1,
	sf::Keyboard::Num2,
	sf::Keyboard::Num3,

	sf::Keyboard::Q,
	sf::Keyboard::W,
	sf::Keyboard::E,
	sf::Keyboard::A,

	sf::Keyboard::S,
	sf::Keyboard::D,
	sf::Keyboard::Z,
	sf::Keyboard::C,

	sf::Keyboard::Num4,
	sf::Keyboard::R,
	sf::Keyboard::F,
	sf::Keyboard::V,
} };

bool logging = false;

std::ostream& nullstream()
{
	class null_buffer : public std::streambuf
	{
	public:
		int overflow(int c) { return c; }
	};
	static null_buffer nbuf{};
	static std::ostream nstream(&nbuf);

	return nstream;
}

std::ostream& logstream()
{
	if(logging) {
		return std::cerr;
	} else {
		return nullstream();
	}
}

uintmax_t byte_concat(std::initializer_list<unsigned char> il)
{
	uintmax_t val = 0;
	for(auto byte : il) {
		val = (val << 8) | byte;
	}
	return val;
}

auto& global_rng()
{
	static randutils::mt19937_rng rng;
	return rng;
}

static const std::array<uint8_t, 80> fontset = { {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	} };

static std::vector<uint8_t> rom;

struct chip8
{

	using rtclock = std::chrono::steady_clock;
	using virtual_ticks = std::chrono::duration<uint8_t, std::ratio<1, 60>>;
	using rt_ticks = rtclock::duration;

public: // variables

	std::array<uint8_t, 0x1000> mem;

	std::bitset<64 * 32> disp;
	bool update_disp;

	std::array<uint8_t, 16> V;
	uint16_t ip; // instruction pointer
	uint16_t I; // index register

	rt_ticks delay_timer;
	rt_ticks sound_timer;

	std::vector<uint16_t> stack; // virtual sp points to last value

	std::bitset<16> keys;
	bool key_stalling; // if waiting for key
	uint8_t keywait_reg;

	rtclock::time_point previous;

public: // methods

	void invalid(const char* s) const
	{
		// this->dump();
		// std::cerr << s << '\n';
		throw std::logic_error(s);
	}

	void init()
	{
		ip = 0x200; // program starts at 0x200
		I = 0; // reset index

		mem.fill(0); // clear memory
		V.fill(0); // clear regs
		stack.clear(); // clear stack

		disp.reset(); // clear display
		update_disp = false;

		keys.reset(); // clear keys
		key_stalling = false;
		keywait_reg = 0;

		// load fontset
		std::copy(fontset.begin(), fontset.end(), mem.begin());

		// reset timers
		delay_timer = rt_ticks::zero();
		sound_timer = rt_ticks::zero();
		previous = rtclock::time_point::min();

		// load rom
		std::copy(rom.begin(), rom.end(), mem.begin() + 0x200);
	}

	bool interpret_op8(uint8_t op, uint8_t a, uint8_t b)
	{
		auto& vf = V[0xf];
		bool flag = vf;

		switch(op) {
		case 0:
			// move
			fmt::print(logstream(), "v{:x} ({:x}) := v{:x} ({:x})\n", a, V[a], b, V[b]);
			V[a] = V[b];
			break;
		case 1:
			// bitwise or
			fmt::print(logstream(), "v{:x} ({:x}) |= v{:x} ({:x})", a, V[a], b, V[b]);
			V[a] |= V[b];
			fmt::print(logstream(), " => {:x}\n", V[a]);
			break;
		case 2:
			// bitwise and
			fmt::print(logstream(), "v{:x} ({:x}) &= v{:x} ({:x})", a, V[a], b, V[b]);
			V[a] &= V[b];
			fmt::print(logstream(), " => {:x}\n", V[a]);
			break;
		case 3:
			// bitwise xor
			fmt::print(logstream(), "v{:x} ({:x}) ^= v{:x} ({:x})", a, V[a], b, V[b]);
			V[a] ^= V[b];
			fmt::print(logstream(), " => {:x}\n", V[a]);
			break;
		case 4:
			// add
			fmt::print(logstream(), "v{:x} ({:x}) += v{:x} ({:x})", a, V[a], b, V[b]);
			flag = V[a] + V[b] > 0xff; // if carry
			V[a] = V[a] + V[b];
			fmt::print(logstream(), " => {:x} ({}vf)\n", V[a], flag ? "" : "!");
			break;
		case 5:
			// subtract
			fmt::print(logstream(), "v{:x} ({:x}) -= v{:x} ({:x})", a, V[a], b, V[b]);
			flag = V[a] > V[b]; // if not borrow
			V[a] = V[a] - V[b];
			fmt::print(logstream(), " => {:x} ({}vf)\n", V[a], flag ? "" : "!");
			break;
		case 6:
			// shift right 1
			fmt::print(logstream(), "v{:x} ({:x}) = v{:x} ({:x}) >> 1", a, V[a], b, V[b]);
			flag = V[b] & 1;
			V[a] = V[b] >> 1;
			fmt::print(logstream(), " => {:x} ({}vf)\n", V[a], flag ? "" : "!");
			break;
		case 7:
			// sub self
			fmt::print(logstream(), "v{0:x} ({1:x}) = v{2:x} ({3:x}) - v{0:x} ({1:x})", a, V[a], b, V[b]);
			flag = V[b] > V[a]; // if not borrow
			V[a] = V[b] - V[a];
			fmt::print(logstream(), " => {:x} ({}vf)\n", V[a], flag ? "" : "!");
			break;
		case 0xE:
			// shift left 1
			fmt::print(logstream(), "v{:x} ({:x}) = v{:x} ({:x}) << 1", a, V[a], b, V[b]);
			flag = V[b] & (1 << 7);
			V[a] = V[b] << 1;
			fmt::print(logstream(), " => {:x} ({}vf)\n", V[a], flag ? "" : "!");
			break;
		default:
			// invalid op
			return false;
		}

		vf = flag;
		return true;
	}

	void draw_sprite(uint8_t x, uint8_t y, uint8_t height)
	{
		// for each row
		for(int yline = 0; yline < height; ++yline) {
			std::bitset<8> pixel = mem[I + yline];
			// for each column
			for(int xline = 0; xline < 8; ++xline) {
				if(!pixel.test(7 - xline)) {
					continue;
				}

				// flip if bit is set
				auto offset = (((x + xline) % 64) + ((y + yline) % 32) * 64) % disp.size();
				if(disp.test(offset)) {
					V[0xf] = 1;
				}
				disp.flip(offset);
			}
		}
	}

	void interpret_op(uint16_t opcode)
	{
		// truncate as necessary
		std::array<uint8_t, 4> op = { {
			(opcode >> 12) & 0xf,
			(opcode >> 8) & 0xf,
			(opcode >> 4) & 0xf,
			(opcode) & 0xf
		} };

		auto& vf = V[0xf];
		bool no_advance = false;

		switch(op[0]) {
		case 0:
			if(opcode == 0x00e0) {
				// clear display
				fmt::print(logstream(), "clear\n");
				disp.reset();
				update_disp = true;
			} else if(opcode == 0x00ee) {
				// return
				fmt::print(logstream(), "ret\n");
				if(stack.empty()) {
					this->invalid("Empty stack when returning");
					return;
				}
				ip = stack.back();
				stack.pop_back();
			} else {
				fmt::print(logstream(), "rca {:x}\n", opcode & 0x0fff);
				// RCA (system) call, not implemented
				this->invalid("RCA call not implemented");
				return;
			}
			break;
		case 1:
			// unconditional jump
			fmt::print(logstream(), "jmp {:x}\n", opcode & 0x0fff);
			ip = opcode & 0x0fff;
			no_advance = true;
			break;
		case 2:
			// call
			fmt::print(logstream(), "call {:x}\n", opcode & 0x0fff);
			stack.push_back(ip);
			ip = opcode & 0x0fff;
			no_advance = true;
			break;
		case 3:
			// skip if Vx == NN
			fmt::print(logstream(), "skip if v{:x} ({:x}) == {:x}\n", op[1], V[op[1]], opcode & 0x00ff);
			if(V[op[1]] == (opcode & 0x00ff)) {
				ip += 2;
			}
			break;
		case 4:
			// skip if Vx != NN
			fmt::print(logstream(), "skip if v{:x} ({:x}) != {:x}\n", op[1], V[op[1]], opcode & 0x00ff);
			if(V[op[1]] != (opcode & 0x00ff)) {
				ip += 2;
			}
			break;
		case 5:
			if(op[3] == 0) {
				fmt::print(logstream(), "skip if v{:x} ({:x}) == v{:x} ({:x})\n", op[1], V[op[1]], op[1], V[op[2]]);
				// skip if Vx == Vy
				if(V[op[1]] == V[op[2]]) {
					ip += 2;
				}
			} else {
				// invalid instruction
				this->invalid("Invalid instruction");
				return;
			}
			break;
		case 6:
			// set to immd
			V[op[1]] = opcode & 0x00ff;
			break;
		case 7:
			// add immd
			V[op[1]] += opcode & 0x00ff;
			break;
		case 8:
			// arithmetic/bitwise ops
			if(!this->interpret_op8(op[3], op[1], op[2])) {
				// invalid instruction
				this->invalid("Invalid instruction");
				return;
			}
			break;
		case 9:
			if(op[3] == 0) {
				// skip if Vx != Vy
				if(V[op[1]] != V[op[2]]) {
					ip += 2;
				}
			} else {
				// invalid instruction
				this->invalid("Invalid instruction");
				return;
			}
			break;
		case 0xA:
			// set index reg
			I = opcode & 0x0fff;
			break;
		case 0xB:
			// jump with offset
			ip = V[0] + (opcode & 0x0fff);
			no_advance = true;
			break;
		case 0xC:
			// random value
			V[op[1]] = global_rng().uniform(0, 0xff) & (opcode & 0x00ff);
			break;
		case 0xD:
			vf = 0;
			this->draw_sprite(V[op[1]], V[op[2]], op[3]);
			update_disp = true;
			break;
		case 0xE:
			if((opcode & 0x00ff) == 0x9e) {
				if(keys.test(V[op[1]])) {
					ip += 2;
				}
			} else if((opcode & 0x00ff) == 0xa1) {
				if(!keys.test(V[op[1]])) {
					ip += 2;
				}
			} else {
				// invalid instruction
				this->invalid("Invalid instruction");
				return;
			}
			break;
		case 0xF:
			switch(opcode & 0x00ff) {
			case 0x07:
				V[op[1]] = std::chrono::duration_cast<virtual_ticks>(delay_timer).count();
				break;
			case 0x0A:
				key_stalling = true;
				keywait_reg = op[1];
				break;
			case 0x15:
				delay_timer = std::chrono::duration_cast<rt_ticks>(virtual_ticks{V[op[1]]});
				break;
			case 0x18:
				sound_timer = std::chrono::duration_cast<rt_ticks>(virtual_ticks{V[op[1]]});
				break;
			case 0x1E:
				I += V[op[1]];
				break;
			case 0x29:
				I = V[op[1]] * 5;
				break;
			case 0x33:
				mem[I + 0] = (V[op[1]] / 100) % 10;
				mem[I + 1] = (V[op[1]] / 10) % 10;
				mem[I + 2] = (V[op[1]] / 1) % 10;
				break;
			case 0x55:
				std::copy(V.begin(), V.begin() + op[1] + 1, mem.begin() + I);
				I += op[1] + 1;
				break;
			case 0x65:
				std::copy(mem.begin() + I, mem.begin() + I + op[1] + 1, V.begin());
				I += op[1] + 1;
				break;
			default:
				// invalid instruction
				this->invalid("Invalid instruction");
				return;
				break;
			}
			break;
		default:
			this->invalid("Invalid instruction");
			return;
			break;
		}

		if(!no_advance) {
			ip += 2;
		}
	}

	void step()
	{
		auto now = rtclock::now();
		auto diff = now - previous;
		previous = now;

		if(delay_timer > rt_ticks::zero()) {
			if(diff >= delay_timer) {
				delay_timer = rt_ticks::zero();
			} else {
				delay_timer -= diff;
			}
		}

		bool beep = false;

		if(sound_timer > rt_ticks::zero()) {
			bool over_1s = sound_timer >= virtual_ticks(1);
			if(diff >= delay_timer) {
				beep = over_1s;
				sound_timer = rt_ticks::zero();
			} else {
				sound_timer -= diff;
				if(over_1s && sound_timer < virtual_ticks(1)) {
					beep = true;
				}
			}
		}

		if(!key_stalling) {
			uint16_t full_op = uint16_t(byte_concat({ mem[ip], mem[ip + 1] }));
			this->interpret_op(full_op);
		}
	}

	void dump() const
	{
		std::cout << "==> memory <==\n";
		for(auto off = 0; off < mem.size(); off += 64) {
			fmt::print("{:04x} | ", off);
			for(auto i = 0; i < 64; ++i) {
				fmt::print("{:02x} ", mem[i + off]);
			}
			fmt::print("|\n");
		}

		std::cout << "==> display <==\n";
		for(auto row = 0; row < 32; ++row) {
			for(auto col = 0; col < 64; ++col) {
				std::cout << (disp.test(col + row * 64) ? '#' : '.');
			}
			std::cout << '\n';
		}

		std::cout << "==> registers <==\n";
		for(auto i = 0; i < 16; ++i) {
			fmt::print("V{:x}: {:x} ", i, V[i]);
		}
		fmt::print("\nI: {:04x} ", I);
		fmt::print("PC: {:04x} [{:04x}]\n", ip, byte_concat({ mem[ip], mem[ip + 1] }));
		std::cout << std::chrono::duration_fmt(std::chrono::symbol)
			<< "delay: " << delay_timer << " (" << std::to_string(std::chrono::duration_cast<virtual_ticks>(delay_timer).count()) << "t) \n";
	}

	void draw()
	{
		if(!update_disp && !rt::opt::a) {
			return;
		}
		update_disp = false;

		stdwin->clear(sf::Color::Black);

		vec2 pixel_size = stdwin.winsize / vec2{ 64, 32 };

		sf::RectangleShape pixel(pixel_size);
		pixel.setFillColor(sf::Color::White);

		for(int y = 0; y < 32; ++y) {
			for(int x = 0; x < 64; ++x) {
				if(disp.test(x + y * 64)) {
					pixel.setPosition(vec2{x, y} * pixel_size);
					stdwin->draw(pixel);
				}
			}
		}
		stdwin->display();
	}

	void breakpoint() const
	{
		// get line
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	void update_keys()
	{
		for(int i = 0; i < 16; ++i) {
			keys.set(i, keybinds[i].pressed());
		}
	}

	void update_keywait(sf::Keyboard::Key key)
	{
		if(!key_stalling) {
			return;
		}

		for(int i = 0; i < 16; ++i) {
			if(keybinds[i].get_key() == key) {
				key_stalling = false;
				V[keywait_reg] = i;
			}
		}
	}
};

namespace var {
	chip8 vm;
	int hz = 20;
}

void initial()
{
	if(rt::args.empty()) {
		std::cout << rt::pgname << ": insufficient arguments\n";
		rt::exit(1);
	} else if(rt::args.size() > 1) {
		std::cout << rt::pgname << ": too many arguments\n";
		rt::exit(1);
	}

	std::ifstream romfile(rt::args[0], std::fstream::binary);
	if(!romfile) {
		std::cout << rt::pgname << ": " << rt::argv[0] << ": cannot open\n";
		rt::exit(1);
	}

	rom.clear();
	for(unsigned char byte = romfile.get(); romfile; byte = romfile.get()) {
		rom.push_back(byte);
	}

	var::vm.init();

	if(rt::opt::c) {
		var::hz = std::stoi(*rt::opt::c);
	}

	rt::on_frame.connect([] {
			try {
				var::vm.update_keys();
				if(rt::opt::b) {
					var::vm.dump();
					var::vm.breakpoint();
				}
				var::vm.step();
				for(int i = 0; i < var::hz - 1 ; ++i) {
					var::vm.step();
				}
				var::vm.draw();
			} catch(...) {
				var::vm.dump();
				throw;
			}
		});

	rt::on_win_event.connect([] (const sf::Event& e) {
			if(e.type == sf::Event::KeyPressed) {
				var::vm.update_keywait(e.key.code);
			}
		});
}
