#include "memfile.hpp"

#include <cassert>
#include <memory>
#include <system_error>

#ifdef _MSC_VER
#include "include/win32.hpp"
#else
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

namespace res {

	ro_memfile::ro_memfile()
		: file(nullptr), mapping(nullptr), fd(-1), view(nullptr), file_size(0)
	{
	}

	ro_memfile::ro_memfile(const char* filename)
		: ro_memfile()
	{
		std::error_code ec;
		if(!this->open(filename, ec)) {
			assert(ec && "error code should not be clear");
			throw std::system_error(ec);
		}

		assert(!ec && "success should have clear exit code");
		assert(this->is_open() && "successful exit should have opened");
	}

	ro_memfile::~ro_memfile()
	{
		this->close();
	}

	bool ro_memfile::open(const char* filename, std::error_code& ec)
		noexcept
	{
		this->close();

#ifdef _MSC_VER
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
#else
		auto fail = [&] {
			ec.assign(errno, std::generic_category());

			if(fd != -1) {
				::close(fd);
			}

			return false;
		};

		fd = ::open(filename, O_RDONLY, 0);
		if(fd == -1) {
			return fail();
		}

		struct stat statdata;
		if(fstat(fd, &statdata) == -1) {
			return fail();
		}

		file_size = statdata.st_size;

		view = mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
		if(view == MAP_FAILED) {
			view = nullptr;
			return fail();
		}

		return true;
#endif
	}

	void ro_memfile::open(const char* filename)
	{
		std::error_code ec;
		if(!this->open(filename, ec)) {
			throw std::system_error(ec.value(), ec.category());
		}
	}

	bool ro_memfile::is_open() const
	{
		return view != nullptr;
	}

	void ro_memfile::close()
	{
		if(this->is_open()) {
#ifdef _MSC_VER
			UnmapViewOfFile(view);
			view = nullptr;
			CloseHandle(mapping);
			mapping = nullptr;
			CloseHandle(file);
			file = nullptr;
#else
			munmap(view, file_size);
			view = nullptr;
			::close(fd);
			fd = -1;
#endif
		}
	}

	const void* ro_memfile::get() const
	{
		return view;
	}

	uint64_t ro_memfile::size() const
	{
		return file_size;
	}

} // namespace res
