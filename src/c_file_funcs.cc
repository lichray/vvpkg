#include <vvpkg/c_file_funcs.h>
#include <vvpkg/fd_funcs.h>

namespace vvpkg
{

#if defined(_WIN32)

FILE* xfopen_wxb(char const* fn)
{
	int fd = xopen_for_write(fn, true);
	return _fdopen(fd, "wb");
}

#endif

#if !defined(_WIN32)

size_t buffer_size_for(FILE* fp) noexcept
{
	return buffer_size_for(fileno(fp));
}

#endif

}
