#if defined(_WIN32)
#include <vvpkg/c_file_funcs.h>
#include <vvpkg/fd_funcs.h>

namespace vvpkg
{

FILE* xfopen_wxb(char const* fn)
{
	int fd = xopen_for_write(fn, true);
	return _fdopen(fd, "wb");
}

}
#endif
