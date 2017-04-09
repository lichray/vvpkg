#include "doctest.h"
#include "testdata.h"

#include <vvpkg/repository.h>
#include <vvpkg/stream_funcs.h>

#include <sstream>
#include <vector>
#include <stdio.h>

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

		::remove("tmp/r2.json");
	}
}

struct pre_imaged_bundle : vvpkg::unmanaged_bundle
{
	template <typename T>
	bool consume(T const& v)
	{
		for (auto&& s : v)
			add_block(s);

		return false;
	}
};

TEST_CASE("revising")
{
	GIVEN("two lists of random blocks")
	{
		std::vector<std::string> r2;
		std::generate_n(std::back_inserter(r2), 10, [] {
			return get_random_text(randint<size_t>(1, 20 * 1024));
		});

		decltype(r2) r1;
		sample(begin(r2), end(r2), std::back_inserter(r1), 7);

		WHEN("committed both")
		{
			pre_imaged_bundle bs;
			auto repo = vvpkg::repository("tmp");

			repo.commit("rv1", bs, r1);
			repo.commit("rv2", bs, r2);

			THEN("both can be retrieved")
			{
				auto repo = vvpkg::repository("tmp", "r");

				auto try_retrieve = [&](auto rev, auto& orig) {
					std::stringstream out;
					auto sz = repo.checkout(
					    rev, vvpkg::to_stream(out));
					auto ns = out.str();

					REQUIRE(sz == ns.size());

					size_t j = 0;
					for (auto&& s : orig)
					{
						REQUIRE(
						    stdex::string_view(ns)
						        .substr(j, s.size()) ==
						    s);
						j += s.size();
					}
				};

				try_retrieve("rv1", r1);
				try_retrieve("rv2", r2);

				::remove("tmp/rv2.json");
				::remove("tmp/rv1.json");
			}
		}
	}
}
