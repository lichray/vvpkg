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

private:
	std::unique_ptr<FILE, c_file_deleter> fp_;
	std::string fn_;
};

}
