/* -*- cpp.doxygen -*- */
#pragma once

#include <cstdint>
#include <system_error>

namespace res {

	/**
	 * \class ro_memfile
	 * \brief Read-only memory-mapped file
	 *
	 * This provides a way to access files as if they were loaded in memory.
	 */
	class ro_memfile
	{
	private: // variables

		void* file;
		void* mapping;

		void* view;
		uint64_t file_size;

	public: // methods

		ro_memfile();
		ro_memfile(const char* filename);

		ro_memfile(const ro_memfile&) = delete;
		ro_memfile& operator=(const ro_memfile&) = delete;

		~ro_memfile();

		// returns if open is successful
		bool open(const char* filename, std::error_code& ec)
			noexcept;

		// throwing overload
		// the function succeeds if it does not throw
		void open(const char* filename);

		bool is_open() const;
		void close();

		const void* get() const;
		uint64_t size() const;

	};

} // namespace res
