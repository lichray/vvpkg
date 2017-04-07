#pragma once

#include <rapidjson/writer.h>

#include <type_traits>

namespace vvpkg
{

template <typename Stream>
struct manifest_creater
{
	template <typename... Args,
	          std::enable_if_t<
	              std::is_constructible<Stream, Args...>::value, int> = 0>
	explicit manifest_creater(Args&&... args)
	    : buffer_(std::forward<Args>(args)...), writer_(buffer_)
	{
		writer_.StartArray();
	}

	template <typename T>
	void append(T const& blockid)
	{
		writer_.Hexlify(blockid);
	}

	void close()
	{
		writer_.EndArray();
	}

	~manifest_creater()
	{
		if (not writer_.IsComplete())
			close();
	}

private:
	struct hexdigest_writer : rapidjson::Writer<Stream>
	{
		explicit hexdigest_writer(Stream& os)
		    : rapidjson::Writer<Stream>(os)
		{
		}

		template <typename T>
		void Hexlify(T const& t)
		{
			this->Prefix(rapidjson::kStringType);
			this->os_->Put('\"');
			hashlib::detail::hexlify_to(
			    t, this->os_->Push(t.size() * 2));
			this->os_->Put('\"');
		}
	};

	Stream buffer_;
	hexdigest_writer writer_;
};

}
