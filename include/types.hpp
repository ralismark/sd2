#pragma once

#include <cstdint>

using std::size_t;

template <size_t N>
using byte_block = const uint8_t[N];
