/* -*- cpp.doxygen -*- */
#pragma once

#include "include/types.hpp"

namespace res {

	/**
	 * \class ro_memblk
	 * \brief Non-owning readable memory block
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

		ro_memblk();
		ro_memblk(const void* addr_init, uint64_t size_init);

		template <uint64_t N>
		ro_memblk(byte_block<N>& block)
			: ro_memblk()
		{
			this->open(block);
		}

		void open(const void* addr_init, uint64_t size_init);

		template <uint64_t N>
		void open(byte_block<N>& block)
		{
			this->open(static_cast<const void*>(&block[0]), N);
		}

		bool is_open() const;
		void close();

		const void* get() const;
		uint64_t size() const;

	};

} // namespace res
