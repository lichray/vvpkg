/*
 * Copyright 2014 Rackspace, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RAX_DEUCECLIENT_BUNDLE_H
#define RAX_DEUCECLIENT_BUNDLE_H

#include "block_arrangement.h"

#include <vector>
#include <tuple>
#include <functional>
#include <cerrno>
#include <system_error>

#include <boost/assert.hpp>

namespace rax
{
namespace deuceclient
{

typedef std::function<size_t(char*, size_t)>	callback;

struct bundle
{
private:
	typedef std::tuple<size_t, sha1_digest>		block_info;
	typedef std::vector<block_info>::const_iterator	block_info_iter;

public:
	bundle();
	explicit bundle(size_t cap);

#if defined(_MSC_VER) && _MSC_VER < 1900
	bundle(bundle&& other);
	bundle& operator=(bundle&& other);
#endif

	bool empty() const;
	size_t size() const;
	size_t capacity() const;

	void clear();

	size_t size_of_block(block_info_iter it) const;
	size_t serialized_size_of_block(block_info_iter it) const;
	void copy_block(block_info_iter it, bundle& bs) const;

	auto blocks() const -> std::vector<block_info> const&;

	size_t serialized_size() const;
	callback get_serializer() const;

protected:
	void mark_new_block(size_t blocksize);

	char* gbase() const;
	char* egptr() const;
	char* egptr(block_info const& info) const;

private:
	static size_t packed_size_of_block_map_header();
	static size_t packed_size_of_block_info();

	size_t cap_;
	std::unique_ptr<char[]> buf_;
	std::vector<block_info> pos_;
};

struct unmanaged_bundle : bundle
{
	template <typename StringLike>
	void add_block(StringLike const& data)
	{
		BOOST_ASSERT_MSG((capacity() - size()) >= data.size(),
		    "bundle size overflow");

#if !defined(_MSC_VER)
		std::copy_n(data.data(), data.size(), egptr());
#else
		std::copy_n(data.data(), data.size(),
		    stdext::make_unchecked_array_iterator(egptr()));
#endif
		mark_new_block(data.size());
	}
};

template <typename Algorithm>
struct managed_bundle : bundle
{
	explicit managed_bundle(size_t cap) :
		bundle(cap),
		pptr_(pbase()),
		epptr_(pbase()),
		needs_reset_(false)
	{}

	template <typename Reader>
	bool consume(Reader&& f)
	{
		std::error_code ec;
		auto is_full = consume(std::forward<Reader>(f), ec);

		if (ec)
#if !(defined(_MSC_VER) && _MSC_VER < 1700)
			throw std::system_error(ec);
#else
			throw std::system_error(ec, ec.message());
#endif

		return is_full;
	}

	template <typename Reader>
	bool consume(Reader&& f, std::error_code& ec)
	{
		if (needs_reset_)
		{
#if !defined(_MSC_VER)
			epptr_ = std::move(pptr_, epptr_, pbase());
#else
			epptr_ = std::move(pptr_, epptr_,
			    stdext::make_unchecked_array_iterator(pbase()))
			    .base();
#endif
			pptr_ = pbase();
			needs_reset_ = false;
		}

		BOOST_ASSERT_MSG(not buffer_is_full(),
		    "buffer size overflow");

		auto len = std::forward<Reader>(f)(epptr_, unused_blen());

		if (len < 0)
		{
			ec.assign(errno, std::system_category());
			return false;
		}

		bool reached_eof = size_t(len) < unused_blen();

		epptr_ += len;
		pptr_ = split_into_blocks(pptr_, epptr_, reached_eof);

		return needs_reset_ = buffer_is_full();
	}

	Algorithm& boundary()
	{
		return algo_;
	}

private:
	size_t unused_blen() const
	{
		return capacity() - (epptr_ - gbase());
	}

	bool buffer_is_full() const
	{
		return unused_blen() == 0;
	}

	char* split_into_blocks(char* first, char* last, bool reached_eof)
	{
		auto lbp = first;

		while (first != last)
		{
			algo_.process_byte(*first++);
			size_t current_size = first - lbp;

			if ((first == last and reached_eof) or
			    algo_.reached_boundary(current_size))
			{
				mark_new_block(current_size);
				first = lbp + current_size;
				lbp = first;
				algo_.reset();
			}
		}

		return lbp;
	}

	char* pbase() const
	{
		return gbase();
	}

	char* pptr_;
	char* epptr_;
	bool needs_reset_;
	Algorithm algo_;
};

inline
bundle::bundle() :
	cap_(5 * 1024 * 1024),
	buf_(new char[capacity()])
{}

inline
bundle::bundle(size_t cap) :
	cap_(cap),
	buf_(new char[capacity()])
{}

#if defined(_MSC_VER) && _MSC_VER < 1900

inline
bundle::bundle(bundle&& other) :
	cap_(std::move(other.cap_)),
	buf_(std::move(other.buf_)),
	pos_(std::move(other.pos_))
{}

inline
bundle& bundle::operator=(bundle&& other)
{
	cap_ = std::move(other.cap_);
	buf_ = std::move(other.buf_);
	pos_ = std::move(other.pos_);

	return *this;
}

#endif

inline
bool bundle::empty() const
{
	return size() == 0;
}

inline
size_t bundle::size() const
{
	return pos_.empty() ? 0 : std::get<0>(pos_.back());
}

inline
size_t bundle::capacity() const
{
	return cap_;
}

inline
void bundle::clear()
{
	pos_.clear();
}

inline
size_t bundle::size_of_block(block_info_iter it) const
{
	BOOST_ASSERT_MSG(pos_.begin() <= it and it < pos_.end(),
	    "block not in this bundle");

	auto eob = std::get<0>(*it);
	return it == pos_.begin() ? eob : eob - std::get<0>(it[-1]);
}

inline
size_t bundle::serialized_size_of_block(block_info_iter it) const
{
	return packed_size_of_block_info() + size_of_block(it);
}

inline
void bundle::copy_block(std::vector<block_info>::const_iterator it,
    bundle& bs) const
{
	auto blksize = size_of_block(it);
	auto first = egptr(*it) - blksize;

#if !defined(_MSC_VER)
	std::copy_n(first, blksize, bs.egptr());
#else
	std::copy_n(first, blksize,
	    stdext::make_unchecked_array_iterator(bs.egptr()));
#endif
	bs.pos_.push_back(std::make_tuple(bs.size() + blksize,
	    std::get<1>(*it)));
}

inline
auto bundle::blocks() const -> std::vector<block_info> const&
{
	return pos_;
}

inline
void bundle::mark_new_block(size_t blocksize)
{
	pos_.push_back(std::make_tuple(size() + blocksize,
	    hashlib::sha1(egptr(), blocksize).digest()));
}

inline
char* bundle::gbase() const
{
	return buf_.get();
}

inline
char* bundle::egptr() const
{
	return gbase() + size();
}

inline
char* bundle::egptr(block_info const& info) const
{
	return gbase() + std::get<0>(info);
}

inline
size_t bundle::serialized_size() const
{
	return packed_size_of_block_map_header() +
	    packed_size_of_block_info() * pos_.size() + size();
}

inline
size_t bundle::packed_size_of_block_map_header()
{
	// (map 16)
	return 3;
}

inline
size_t bundle::packed_size_of_block_info()
{
	// (str 8) + hex + (bin 32)
	return 2 + (std::tuple_size<sha1_digest>::value * 2) + 5;
}

}
}

#endif
