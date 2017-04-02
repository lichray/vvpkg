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

TEST_CASE("vfile ctor")
{
	vvpkg::vfile f("tmp");
	_unlink("tmp/vvpkg.db");
	_rmdir("tmp");
}
