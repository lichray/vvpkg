#include <random>
#include <array>
#include <algorithm>
#include <iterator>
#include <functional>
#include <string>
#include <sstream>
#include <ostream>
#include <iomanip>
#include <cstring>

extern std::mt19937 e;

inline
auto get_random_block() -> std::array<char, 4096>
{
	std::array<char, 4096> arr;

	auto it = reinterpret_cast<decltype(e())*>(arr.data());
	auto n = arr.size() / sizeof(decltype(e()));

#if !defined(_MSC_VER)
	std::generate_n(it, n, std::ref(e));
#else
	std::generate_n(stdext::make_unchecked_array_iterator(it), n,
	                std::ref(e));
#endif

	return arr;
}

template <typename ForwardIt, typename OutputIt, typename Size, class URNG>
inline
OutputIt sample(ForwardIt first, ForwardIt last, OutputIt d_first,
    Size n, URNG&& g)
{
	typedef std::uniform_int_distribution<Size>	dist_t;
	typedef typename dist_t::param_type		param_t;

	static dist_t d;

	Size unsampled = static_cast<Size>(std::distance(first, last));

	for (n = std::min(n, unsampled); n != 0; ++first)
	{
		if (d(g, param_t(0, --unsampled)) < n)
		{
			*d_first++ = *first;
			--n;
		}
	}

	return d_first;
}

template <typename ForwardIt, typename OutputIt, typename Size>
inline
OutputIt sample(ForwardIt first, ForwardIt last, OutputIt d_first, Size n)
{
	return ::sample(first, last, d_first, n, e);
}

template <typename IntType>
inline
IntType randint(IntType a, IntType b)
{
	typedef std::uniform_int_distribution<IntType>	dist_t;
	typedef typename dist_t::param_type		param_t;

	static dist_t d;

	return d(e, param_t(a, b));
}

inline
auto get_random_text(size_t len, std::string const& from =
    "-0123456789abcdefghijklmnopqrstuvwxyz")
	-> std::string
{
	std::string to;
	to.resize(len);

	std::generate(begin(to), end(to),
	    [&]()
	    {
		return from[randint(size_t(0), from.size() - 1)];
	    });

	return to;
}

class randombuf : public std::streambuf
{
public:
	randombuf() : len_(-1)
	{}

	explicit randombuf(long long len) : len_(len)
	{}

protected:
	int_type underflow()
	{
		if (len_ == 0)
			return traits_type::eof();

		if (gptr() == egptr())
			refill_buffer();

		return traits_type::to_int_type(*gptr());
	}

private:
	void refill_buffer()
	{
		arr_ = get_random_block();
		auto p = arr_.data();

		if (len_ == -1)
			setg(p, p, p + arr_.size());
		else
		{
			auto sz = std::min(
			    len_, static_cast<long long>(arr_.size()));
			len_ -= sz;
			setg(p, p, p + sz);
		}
	}

	long long len_;
	decltype(get_random_block()) arr_;
};

class randomstream : public std::istream
{
public:
	randomstream() :
		std::ios(nullptr),
		std::istream(&buf_),
		buf_(-1)
	{}

	explicit randomstream(long long len) :
		std::ios(nullptr),
		std::istream(&buf_),
		buf_(len)
	{}

	randombuf* rdbuf() const
	{
		return const_cast<randombuf*>(&buf_);
	}

private:
	randombuf buf_;
};
