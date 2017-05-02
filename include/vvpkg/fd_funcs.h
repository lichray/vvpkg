#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <io.h>
#include <share.h>
#else
#include <sys/param.h>
#include <unistd.h>
#endif

#include <system_error>

namespace vvpkg
{

inline int xopen_for_read(char const* filename)
{
#if defined(_WIN32)
	int fd;
	_sopen_s(&fd, filename, _O_RDONLY | _O_BINARY, _SH_DENYWR, 0);
#else
	auto fd = ::open(filename, O_RDONLY);
#endif

	if (fd == -1)
		throw std::system_error(errno, std::system_category());

	return fd;
}

inline int xopen_for_write_only(char const* filename)
{
#if defined(_WIN32)
	int fd;
	_sopen_s(&fd, filename, _O_WRONLY | _O_BINARY, _SH_DENYRW, 0);
#else
	auto fd = ::open(filename, O_WRONLY);
#endif

	if (fd == -1)
		throw std::system_error(errno, std::system_category());

	return fd;
}

inline int xopen_for_write(char const* filename, int excl = false)
{
#if defined(_WIN32)
	int fd;
	_sopen_s(&fd, filename,
	         _O_CREAT | _O_WRONLY | (excl ? _O_EXCL : _O_TRUNC) |
	             _O_BINARY,
	         _SH_DENYWR, _S_IREAD | _S_IWRITE);
#else
	auto fd =
	    ::open(filename, O_CREAT | O_WRONLY | (excl ? O_EXCL : O_TRUNC),
	           S_IRUSR | S_IWUSR | S_IRGRP);
#endif

	if (fd == -1)
		throw std::system_error(errno, std::system_category());

	return fd;
}

inline int xstdin_fileno()
{
#if defined(_WIN32)
	if (_setmode(0, _O_BINARY) == -1)
		throw std::system_error(errno, std::system_category());
#endif
	return 0;
}

inline int xstdout_fileno()
{
#if defined(_WIN32)
	if (_setmode(1, _O_BINARY) == -1)
		throw std::system_error(errno, std::system_category());
#endif
	return 1;
}

inline int xclose(int fd) noexcept
{
	return
#if defined(_WIN32)
	    _close(fd);
#else
	    ::close(fd);
#endif
}

inline void xlseek(int fd, int64_t offset)
{
#if defined(_WIN32)
	if (_lseeki64(fd, offset, SEEK_SET) == -1)
#elif defined(BSD) || defined(__MSYS__)
	if (::lseek(fd, offset, SEEK_SET) == -1)
#else
	if (::lseek64(fd, offset, SEEK_SET) == -1)
#endif
		throw std::system_error(errno, std::system_category());
}

inline size_t buffer_size_for(int fd) noexcept
{
#if defined(_WIN32)
	return 65536;
#else
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
#endif
}

inline auto from_descriptor(int fd)
{
	return [=](char* p, size_t sz) {
#if defined(_WIN32)
		return _read(fd, p, unsigned(sz));
#else
		return ::read(fd, p, sz);
#endif
	};
}

inline auto to_descriptor(int fd)
{
	return [=](char const* p, size_t sz) {
#if defined(_WIN32)
		return _write(fd, p, unsigned(sz));
#else
		return ::write(fd, p, sz);
#endif
	};
}

}
