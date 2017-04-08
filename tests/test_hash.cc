#include "doctest.h"

#include <deuceclient/hashlib.h>
#include <ostream>

using namespace stdex::literals;

TEST_CASE("blake2b_160")
{
	hashlib::blake2b_160 h;
	REQUIRE(h.hexdigest() == "3345524abf6bbe1809449224b5972c41790b6cf2"_sv);

	h.update("Hello world\n");
	REQUIRE(h.hexdigest() == "dd0db13545cda280d0b160871b91cbd78712b806"_sv);
}

TEST_CASE("blake2s_160")
{
	hashlib::blake2s_160 h;
	REQUIRE(h.hexdigest() == "354c9c33f735962418bdacb9479873429c34916f"_sv);

	h.update("Hello world\n");
	REQUIRE(h.hexdigest() == "44a14f7e45c69dfa7cfc7f26d93555c9a7b8ed26"_sv);
}
