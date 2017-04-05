#pragma once

#include "vfile.h"
#include "sync_store.h"

namespace vvpkg
{

template <typename Store>
struct basic_vfile_packager
{
	explicit basic_vfile_packager(std::string path)
	    : f_(path), store_(std::move(path) + "/vvpkg.bin")
	{
	}

	template <typename Bundle, typename Reader>
	int64_t commit(std::string commitid, Bundle& bs, Reader&& f)
	{
		int64_t file_size = 0;
		bool bundle_is_full;
		auto r1 = f_.new_revision(std::move(commitid));

		do
		{
			bundle_is_full = bs.consume(std::forward<Reader>(f));

			if (bs.empty())
				break;

			f_.merge(r1.assign_blocks(bs), bs, store_);
			file_size += int64_t(bs.size());
			bs.clear();

		} while (bundle_is_full);

		return file_size;
	}

private:
	vfile f_;
	Store store_;
};

using vfile_packager = basic_vfile_packager<sync_store>;

}
