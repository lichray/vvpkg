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

#ifndef RAX_DEUCECLIENT_BLOCK__ARRANGEMENT_H
#define RAX_DEUCECLIENT_BLOCK__ARRANGEMENT_H

#include "hashlib.h"

#include <memory>
#include <cstdint>
#include <type_traits>
#include <limits>

namespace rax
{
namespace deuceclient
{

using hash =
    std::conditional_t<(std::numeric_limits<std::uintptr_t>::digits < 64),
                       hashlib::blake2s_160, hashlib::blake2b_160>;
using msg_digest = hash::digest_type;

struct block_arrangement
{
	block_arrangement();
	~block_arrangement();

	block_arrangement(block_arrangement&& other);
	block_arrangement& operator=(block_arrangement&& other);

	void add(msg_digest blockid, int64_t offset);
	void clear();

	stdex::string_view text();

private:
	struct impl;

	std::unique_ptr<impl> impl_;
};

}
}

#endif
