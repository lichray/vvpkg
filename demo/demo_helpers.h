#include <stdex/defer.h>

#include <cstdlib>
#include <iostream>

inline
std::string getenv_or(char const* name, char const* fallback)
{
#if defined(_WIN32)
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
