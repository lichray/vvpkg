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
		Store store(binfile_location());
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

	template <typename Writer>
	int64_t checkout(std::string commitid, Writer&& f)
	{
		int64_t file_size = 0;
		auto g = f_.list(std::move(commitid));

		std::unique_ptr<FILE, c_file_deleter> fp(
		    xfopen(binfile_location().data(), "rb"));
		// should optimize I/O size for write instead of read
		auto buflen = buffer_size_for(fp.get());
		auto buf = std::make_unique<char[]>(buflen);
		auto p = buf.get();
		auto ep = p + buflen;
		auto do_flush = [&] {
			auto len = size_t(p - buf.get());
			auto n = std::forward<Writer>(f)(buf.get(), len);
			if (n < 0 or size_t(n) != len)
				throw std::system_error(
				    errno, std::system_category());
			p = buf.get();
		};

		for (;;)
		{
			auto off = g();
			auto sz = off.second - off.first;
			if (sz == 0)
				break;
			xfseek(fp.get(), off.first);
			file_size += sz;

			do
			{
				auto len =
				    size_t(std::min<int64_t>(ep - p, sz));
				auto n = from_c_file(fp.get())(p, len);
				if (n != len)
				{
					if (::ferror(fp.get()))
						throw std::system_error(
						    errno,
						    std::system_category());
					else
						throw std::runtime_error(
						    "binfile encountered "
						    "unexpected end-of-file");
				}

				p += n;
				sz -= int64_t(n);

				if (p == ep)
					do_flush();

			} while (sz != 0);
		}

		do_flush();

		return file_size;
	}

private:
	std::string binfile_location() const
	{
		return f_.id().to_string() + "/vvpkg.bin";
	}

	vfile f_;
};

using repository = basic_repository<sync_store>;

}
