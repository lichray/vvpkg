#pragma once

#include "c_file_funcs.h"

#if !defined(_WIN32)
#include <sys/param.h>
#include <sys/stat.h>
#endif
#include <memory>
#include <string>

namespace vvpkg
{

struct sync_store
{
	explicit sync_store(std::string fn) : fn_(std::move(fn)) {}

	void operator()(int64_t from, char const* p, size_t sz)
	{
		if (fp_ == nullptr)
		{
			if (from == 0)
				fp_.reset(xfopen(fn_.data(), "wb"));
			else
			{
				fp_.reset(xfopen(fn_.data(), "r+b"));
				xfseek(fp_.get(), from);
			}
		}

		if (::fwrite(p, 1, sz, fp_.get()) != sz)
			throw std::system_error(errno, std::system_category());
	}

	static size_t buffer_size_for(FILE* fp)
	{
#if defined(_WIN32)
		return 65536;
#else
		return buffer_size_for(fileno(fp));
#endif
	}

	static size_t buffer_size_for(int fd)
	{
		constexpr size_t default_buffer_size = 8192;
#if defined(_WIN32)
		return 65536;
#else
#if defined(BSD) || defined(__MSYS__)
		struct ::stat st;
		int rt = ::fstat(fd, &st);
#else
		struct ::stat64 st;
		int rt = ::fstat64(fd, &st);
#endif
		if (rt == 0 and 0 < st.st_blksize)
			return size_t(st.st_blksize);
		else
			return default_buffer_size;
#endif
	}

private:
	std::unique_ptr<FILE, c_file_deleter> fp_;
	std::string fn_;
};

}
