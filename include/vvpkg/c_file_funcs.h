#pragma once

#include <stdio.h>
#include <system_error>

namespace vvpkg
{

inline FILE* xfopen(char const* fn, char const* mode)
{
#if !defined(_WIN32)
	auto fp = ::fopen(fn, mode);
#else
	FILE* fp;
	fopen_s(&fp, fn, mode);
#endif
	if (fp == nullptr)
		throw std::system_error(errno, std::system_category());

	return fp;
}

extern FILE* xfopen_wxb(char const* fn);

#if !defined(_WIN32)
inline FILE* xfopen_wxb(char const* fn)
{
	return xfopen(fn, "wxb");
}
#endif

#if defined(_WIN32)
inline FILE* xfopen(wchar_t const* fn, wchar_t const* mode)
{
	FILE* fp;
	_wfopen_s(&fp, fn, mode);

	if (fp == nullptr)
		throw std::system_error(errno, std::system_category());

	return fp;
}
#endif

struct c_file_deleter
{
	void operator()(FILE* fp) const { ::fclose(fp); }
};

inline void xfseek(FILE* fp, int64_t offset)
{
#if !defined(_WIN32)
	if (::fseeko(fp, offset, SEEK_SET) == -1)
#else
	if (_fseeki64(fp, offset, SEEK_SET) == -1)
#endif
		throw std::system_error(errno, std::system_category());
}

extern size_t buffer_size_for(FILE* fp) noexcept;

#if defined(_WIN32)
inline size_t buffer_size_for(FILE* fp) noexcept
{
	return 65536;
}
#endif

inline auto from_c_file(FILE* stream)
{
	return [=](char* dst, size_t sz) mutable
	{
		return ::fread(dst, 1, sz, stream);
	};
}

inline auto to_c_file(FILE* stream)
{
	return [=](char const* dst, size_t sz) mutable
	{
		return ::fwrite(dst, 1, sz, stream);
	};
}

}
