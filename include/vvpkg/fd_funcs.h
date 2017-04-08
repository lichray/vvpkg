#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <io.h>
#include <share.h>
#else
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

inline int xopen_for_write(char const* filename)
{
#if defined(_WIN32)
	int fd;
	_sopen_s(&fd, filename, _O_CREAT | _O_WRONLY | _O_BINARY, _SH_DENYWR,
	         _S_IREAD | _S_IWRITE);
#else
	auto fd = ::open(filename, O_CREAT | O_WRONLY | O_TRUNC,
	                 S_IRUSR | S_IWUSR | S_IRGRP);
#endif

	if (fd == -1)
		throw std::system_error(errno, std::system_category());

	return fd;
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
