#include "doctest.h"
#include "testdata.h"

#include <vvpkg/vfile.h>
#if defined(_WIN32)
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#define _unlink ::unlink
#define _rmdir ::rmdir
#endif
#include <stdex/defer.h>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>

TEST_CASE("vfile flow")
{
	defer(_unlink("tmp/vvpkg.db"); _unlink("tmp/r1.json"); _rmdir("tmp"));

	vvpkg::vfile f("tmp");

	auto v = [&] {
		auto r1 = f.new_revision("r1");
		vvpkg::unmanaged_bundle bs;

		auto blk = get_random_block();
		bs.add_block(blk);
		for (int n = 0; n < 9; ++n)
		{
			bs.add_block(get_random_block());
		}
		// duplicates
		bs.add_block(blk);

		return r1.assign_blocks(bs);
	}();
	REQUIRE(v.size() == 10);

	using namespace rapidjson;

	std::ifstream fl("tmp/r1.json");
	REQUIRE(fl.is_open());

	Document d;
	IStreamWrapper fr(fl);
	d.ParseStream(fr);

	REQUIRE(d.IsArray());
	REQUIRE(d.Size() == 11);
}
