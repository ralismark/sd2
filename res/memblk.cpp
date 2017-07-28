#include "memblk.hpp"

namespace res {

	// class ro_memblk {{{

	ro_memblk::ro_memblk()
		: address(nullptr), blk_size(0)
	{
	}

	ro_memblk::ro_memblk(const void* addr_init, uint64_t size_init)
		: ro_memblk()
	{
		this->open(addr_init, size_init);
	}

	void ro_memblk::open(const void* addr_init, uint64_t size_init)
	{
		address = addr_init;
		blk_size = size_init;
	}

	bool ro_memblk::is_open() const
	{
		return address != nullptr;
	}

	void ro_memblk::close()
	{
		address = nullptr;
		blk_size = 0;
	}

	const void* ro_memblk::get() const
	{
		return address;
	}

	uint64_t ro_memblk::size() const
	{
		return blk_size;
	}

	// }}}

} // namespace res
