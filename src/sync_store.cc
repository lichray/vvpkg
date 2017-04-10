#if !defined(_WIN32)
#include <sys/param.h>
#include <sys/stat.h>

#include <vvpkg/sync_store.h>

namespace vvpkg
{

size_t sync_store::buffer_size_for(int fd)
{
	constexpr size_t default_buffer_size = 8192;
#if defined(BSD) || defined(__MSYS__)
	struct ::stat st;
	int rt = ::fstat(fd, &st);
#else
	struct ::stat64 st;
	int rt = ::fstat64(fd, &st);
#endif
	if (rt == 0 and 0 < st.st_blksize)
		return size_t(st.st_blksize);
	else
		return default_buffer_size;
}

}

#endif
