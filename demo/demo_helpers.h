#include <vvpkg/vvpkg.h>

#include <cstdlib>
#include <iostream>
#include <system_error>

#include "defer.h"

#if !(defined(_MSC_VER) && _MSC_VER < 1700)
#define THROW_ERRNO() throw std::system_error(errno, std::system_category())
#else
#define THROW_ERRNO() do {						\
	std::error_code ec(errno, std::system_category());		\
	throw std::system_error(ec, ec.message());			\
} while(0)
#endif

inline
std::string getenv_or(char const* name, char const* fallback)
{
#if defined(WIN32)
	size_t sz;
	getenv_s(&sz, nullptr, 0, name);

	if (sz == 0)
		return fallback;
	else
	{
		std::string s;
		s.resize(sz);
		getenv_s(&sz, &*s.begin(), sz, name);
		s.pop_back();

		return s;
	}
#else
	auto p = getenv(name);

	return p != nullptr ? p : fallback;
#endif
}
