/* -*- cpp.doxygen -*- */
#pragma once

#include <cassert>

#include "include/types.hpp"

#include "except.hpp"

namespace res {

	/**
	 * \class ro_memblk
	 * \brief non-owning readable memory block
	 *
	 * This is intended to be a raw resource similar to ro_memfile, but for
	 * existing memory blocks. As a result, it can be used as a stand-in
	 * replacement if needed.
	 */
	class ro_memblk
	{
	private: // variables
		const void* address;
		uint64_t blk_size;
	public: // methods
		ro_memblk()
			: address(nullptr), blk_size(0)
		{
		}

		ro_memblk(const void* addr_init, uint64_t size_init)
			: ro_memblk()
		{
			this->open(addr_init, size_init);
		}

		template <uint64_t N>
		ro_memblk(byte_block<N>& block)
			: ro_memblk()
		{
			this->open(block);
		}

		void open(const void* addr_init, uint64_t size_init)
		{
			address = addr_init;
			blk_size = size_init;
		}

		template <uint64_t N>
		void open(byte_block<N>& block)
		{
			this->open(static_cast<const void*>(&block[0]), N);
		}

		bool is_open() const
		{
			return address != nullptr;
		}

		void close()
		{
			address = nullptr;
			blk_size = 0;
		}

		const void* get() const
		{
			if(!this->is_open()) {
				throw res::unavailable("resource not loaded");
			}
			return address;
		}

		uint64_t size() const
		{
			return blk_size;
		}
	};

} // namespace res
