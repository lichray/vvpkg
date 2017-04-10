#pragma once

#include "c_file_funcs.h"

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

	static size_t buffer_size_for(int fd);

private:
	std::unique_ptr<FILE, c_file_deleter> fp_;
	std::string fn_;
};

#if defined(_WIN32)
inline size_t sync_store::buffer_size_for(int fd)
{
	return 65536;
}
#endif

}
