/* -*- cpp.doxygen -*- */
#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <system_error>

#include "include/win32.hpp"

#include "except.hpp"

namespace res {

	/**
	 * \class ro_memfile
	 * \brief read-only memory-mapped file
	 *
	 * This provides a way to access files as if they were loaded in memory.
	 */
	class ro_memfile
	{
	private: // variables

		HANDLE file;
		HANDLE mapping;

		void* view;
		uint64_t file_size;

	public: // methods

		ro_memfile()
			: file(nullptr), mapping(nullptr), view(nullptr), file_size(0)
		{
		}

		ro_memfile(const char* filename)
			: ro_memfile()
		{
			std::error_code ec;
			if(!this->open(filename, ec)) {
				assert(ec && "error code should not be clear");
				throw res::unavailable(ec.message());
			}

			assert(!ec && "success should have clear exit code");
			assert(this->is_open() && "successful exit should have opened");
		}

		ro_memfile(const ro_memfile&) = delete;
		ro_memfile& operator=(const ro_memfile&) = delete;

		~ro_memfile()
		{
			this->close();
		}

		// returns if open is successful
		bool open(const char* filename, std::error_code& ec)
			noexcept
		{
			this->close();

			auto fail = [&] {
				ec.assign(GetLastError(), std::system_category());
				return false;
			};

			using raii_handle = std::unique_ptr<void, decltype(&CloseHandle)>;

			raii_handle file_hdl{nullptr, CloseHandle};
			raii_handle mapping_hdl{nullptr, CloseHandle};

			// open file
			file_hdl.reset(CreateFileA(filename, GENERIC_READ,
			                           FILE_SHARE_READ, nullptr,
			                           OPEN_EXISTING,
			                           FILE_ATTRIBUTE_NORMAL, nullptr));
			if(file_hdl.get() == INVALID_HANDLE_VALUE) {
				file_hdl.release(); // INVALID_HANDLE_VALUE != nullptr
				return fail();
			}

			// get (future) memory section size
			LARGE_INTEGER fsize;
			if(!GetFileSizeEx(file_hdl.get(), &fsize)) {
				return fail();
			}
			file_size = static_cast<uint64_t>(fsize.QuadPart);

			// get mapping
			mapping_hdl.reset(CreateFileMappingA(file_hdl.get(), nullptr, PAGE_READONLY, 0, 0, nullptr));
			if(mapping_hdl == nullptr) {
				return fail();
			}

			// get memory section
			view = MapViewOfFile(mapping_hdl.get(), FILE_MAP_READ, 0, 0, 0);
			if(view == nullptr) {
				return fail();
			}

			// all are successful
			ec.clear();
			file = file_hdl.release();
			mapping = mapping_hdl.release();
			return true;
		}

		// throwing overload
		// the function succeeds if it does not throw
		void open(const char* filename)
		{
			std::error_code ec;
			if(!this->open(filename, ec)) {
				throw std::system_error(ec.value(), ec.category());
			}
		}

		bool is_open() const
		{
			return view != nullptr;
		}

		void close()
		{
			if(this->is_open()) {
				UnmapViewOfFile(view);
				view = nullptr;
				CloseHandle(mapping);
				mapping = nullptr;
				CloseHandle(file);
				file = nullptr;
			}
		}

		const void* get() const
		{
			if(!this->is_open()) {
				throw res::unavailable("resource not loaded");
			}
			return view;
		}

		uint64_t size() const
		{
			return file_size;
		}

	};

} // namespace res
