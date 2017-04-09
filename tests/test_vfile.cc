#include "doctest.h"
#include "testdata.h"

#include <vvpkg/vfile.h>
#include <vvpkg/sync_store.h>
#include <stdex/defer.h>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>

TEST_CASE("vfile flow")
{
	defer(::remove("tmp/r1.json"));

	vvpkg::vfile f("tmp");
	vvpkg::sync_store bin("tmp/vvpkg.bin");

	{
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

		auto v = r1.assign_blocks(bs);
		REQUIRE(v.size() == 10);

		f.merge(v, bs, bin);
		v = r1.assign_blocks(bs);
		REQUIRE(v.empty());
	}

	using namespace rapidjson;

	std::ifstream fl("tmp/r1.json");
	REQUIRE(fl.is_open());

	Document d;
	IStreamWrapper fr(fl);
	d.ParseStream(fr);

	REQUIRE(d.IsArray());
	REQUIRE(d.Size() == 22);

	{
		auto f = vvpkg::vfile("tmp", "r");
		auto g = f.list("r1");
		int64_t file_size = 0;

		for (;;)
		{
			auto off = g();
			if (auto sz = (off.second - off.first))
				file_size += sz;
			else
				break;
		}

		REQUIRE(file_size == d.Size() * 4096);
	}
}
