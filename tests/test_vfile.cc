#include "doctest.h"

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

TEST_CASE("vfile ctor")
{
	defer(_unlink("tmp/vvpkg.db"); _rmdir("tmp"));

	vvpkg::vfile f("tmp");
	auto r1 = f.new_revision("r1");
}
