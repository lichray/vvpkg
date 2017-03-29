#include "doctest.h"

#include <deuceclient/hashlib.h>

TEST_CASE("blake2b_160")
{
	hashlib::blake2b_160 h;
	REQUIRE(h.hexdigest() == "3345524abf6bbe1809449224b5972c41790b6cf2");

	h.update("Hello world\n");
	REQUIRE(h.hexdigest() == "dd0db13545cda280d0b160871b91cbd78712b806");
}
