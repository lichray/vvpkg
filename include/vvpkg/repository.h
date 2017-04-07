#pragma once

#include "vfile.h"
#include "sync_store.h"

namespace vvpkg
{

template <typename Store>
struct basic_repository
{
	template <typename... Args,
	          std::enable_if_t<
	              std::is_constructible<vfile, Args...>::value, int> = 0>
	explicit basic_repository(Args&&... args)
	    : f_(std::forward<Args>(args)...)
	{
	}

	template <typename Bundle, typename Reader>
	int64_t commit(std::string commitid, Bundle& bs, Reader&& f)
	{
		auto r1 = f_.new_revision(std::move(commitid));
		Store store(f_.id().to_string() + "/vvpkg.bin");
		int64_t file_size = 0;
		bool bundle_is_full;

		do
		{
			bundle_is_full = bs.consume(std::forward<Reader>(f));

			if (bs.empty())
				break;

			f_.merge(r1.assign_blocks(bs), bs, store);
			file_size += int64_t(bs.size());
			bs.clear();

		} while (bundle_is_full);

		return file_size;
	}

private:
	vfile f_;
};

using repository = basic_repository<sync_store>;

}
