#include "doctest.h"
#include "testdata.h"

#include <vvpkg/repository.h>
#include <vvpkg/stream_funcs.h>

#include <sstream>

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

TEST_CASE("version control flow")
{
	int target_file_size = 128;

	SUBCASE("commit")
	{
		byte_per_byte_bundle bs;
		auto repo = vvpkg::repository("tmp");

		randomstream src(target_file_size);

		auto file_size =
		    repo.commit("r2", bs, vvpkg::from_stream(src));

		REQUIRE(file_size == target_file_size);
	}

	SUBCASE("checkout")
	{
		auto repo = vvpkg::repository("tmp", "r");
		std::stringstream out;

		auto file_size = repo.checkout("r2", vvpkg::to_stream(out));

		REQUIRE(file_size == target_file_size);
		REQUIRE(out.str().size() == file_size);
	}
}
