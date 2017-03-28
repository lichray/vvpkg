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

#include <rapidjson/writer.h>

#include <deuceclient/block_arrangement.h>

namespace rax
{
namespace deuceclient
{

using namespace rapidjson;

struct hexdigest_writer : Writer<StringBuffer>
{
	explicit hexdigest_writer(StringBuffer& os) :
		Writer<StringBuffer>(os)
	{}

	template <typename T>
	void Hexlify(T t)
	{
		Prefix(kStringType);
		os_->Put('\"');
		hashlib::detail::hexlify_to(t, os_->Push(t.size() * 2));
		os_->Put('\"');
	}
};

struct block_arrangement::impl
{
	impl() : writer(buffer) {}

	StringBuffer buffer;
	hexdigest_writer writer;
};

block_arrangement::block_arrangement() :
	impl_(new impl)
{
	impl_->writer.StartArray();
}

block_arrangement::block_arrangement(block_arrangement&& other) :
	impl_(std::move(other.impl_))
{}

block_arrangement& block_arrangement::operator=(block_arrangement&& other)
{
	impl_ = std::move(other.impl_);

	return *this;
}

block_arrangement::~block_arrangement()
{}

void block_arrangement::add(sha1_digest blockid, int64_t offset)
{
	impl_->writer.StartArray();
	impl_->writer.Hexlify(blockid);
	impl_->writer.Int64(offset);
	impl_->writer.EndArray();
}

stdex::string_view block_arrangement::text()
{
	if (not impl_->writer.IsComplete())
		impl_->writer.EndArray();

	return stdex::string_view(impl_->buffer.GetString(),
	    impl_->buffer.GetSize());
}

void block_arrangement::clear()
{
	impl_->buffer.Clear();
	impl_->writer.Reset(impl_->buffer);
	impl_->writer.StartArray();
}

}
}
