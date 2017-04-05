#pragma once

#include <istream>
#include <ostream>

namespace vvpkg
{

inline auto from_stream(std::istream& is)
{
	return [p = is.rdbuf()](char* dst, size_t sz) mutable
	{
		return static_cast<size_t>(p->sgetn(dst, std::streamsize(sz)));
	};
}

inline auto to_stream(std::ostream& os)
{
	return [p = os.rdbuf()](char const* dst, size_t sz) mutable
	{
		return static_cast<size_t>(p->sputn(dst, std::streamsize(sz)));
	};
}

}
