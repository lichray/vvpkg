#include "doctest.h"
#include "testdata.h"

#include <vvpkg/vfile_packager.h>
#include <vvpkg/stream_funcs.h>

struct byte_per_byte_bundle : vvpkg::bundle
{
	template <typename Reader>
	bool consume(Reader&& f)
	{
		auto len =
		    std::forward<Reader>(f)(egptr(), capacity() - size());

		while (len--)
			mark_new_block(1);

		return size() == capacity();
	}
};

TEST_CASE("vfile_packager flow")
{
	byte_per_byte_bundle bs;

	auto pkg = vvpkg::vfile_packager("tmp");

	int target_file_size = 128;
	randomstream src(target_file_size);

	auto file_size = pkg.commit("r2", bs, vvpkg::from_stream(src));

	REQUIRE(file_size == target_file_size);
}
