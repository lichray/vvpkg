#pragma once

#include <rapidjson/reader.h>
#include <rapidjson/error/en.h>

namespace vvpkg
{
namespace detail
{
using namespace rapidjson;

template <typename F>
struct string_list_handler : BaseReaderHandler<UTF8<>, string_list_handler<F>>
{
	explicit string_list_handler(F f) : in_array_(false), f_(std::move(f))
	{
	}

	bool StartArray()
	{
		if (in_array_)
			return false;
		else
		{
			in_array_ = true;
			return true;
		}
	}

	bool EndArray(SizeType) { return true; }

	bool String(char const* str, SizeType length, bool)
	{
		return in_array_ ? (f_(str, length), true) : false;
	}

	bool Default() { return false; }

private:
	bool in_array_;
	F f_;
};

template <typename F>
inline auto make_string_list_handler(F&& f)
{
	return string_list_handler<F>(std::forward<F>(f));
}
}

template <typename Stream>
struct manifest_parser
{
	template <typename... Args>
	explicit manifest_parser(Args&&... args)
	    : ss_(std::forward<Args>(args)...)
	{
	}

	template <typename F>
	void parse(F&& f)
	{
		auto h = detail::make_string_list_handler(std::forward<F>(f));
		constexpr auto options = rapidjson::kParseStopWhenDoneFlag;
		if (not reader_.Parse<options>(ss_, h))
			throw std::invalid_argument(
			    GetParseError_En(reader_.GetParseErrorCode()));
	}

private:
	Stream ss_;
	rapidjson::Reader reader_;
};

}
