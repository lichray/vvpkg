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

#ifndef RAX_RABIN_FINGERPRINT_H
#define RAX_RABIN_FINGERPRINT_H

#include <cstdint>
#include <algorithm>

#if defined(_MSC_VER)
#pragma warning(disable: 4351)
#endif

namespace rax
{

uint64_t _polymmult(uint64_t x, uint64_t y, uint64_t d);

struct rabin_polynomial
{
	explicit rabin_polynomial(uint64_t poly);

	uint64_t add_byte(uint64_t p, uint8_t m)
	{
		return ((p << 8) | m) ^ T[p >> shift];
	}

private:
	void calculate_lookup_table();

	int shift;
	uint64_t T[256];
	uint64_t const poly;
};

template <int WindowSize>
struct rabin_fingerprint : private rabin_polynomial
{
	static const auto window_size = WindowSize;

	explicit rabin_fingerprint(uint64_t poly) :
		rabin_polynomial(poly),
		bufpos(0),
		buf(),
		fingerprint(0)
	{
		uint64_t sizeshift = 1;

		for (int i = 1; i < window_size; i++)
			sizeshift = add_byte(sizeshift, 0);

		for (int i = 0; i < 256; i++)
			U[i] = _polymmult(i, sizeshift, poly);
	}

	void process_byte(uint8_t m)
	{
		auto om = buf[bufpos];
		buf[bufpos] = m;

		fingerprint = add_byte(fingerprint ^ U[om], m);

		if (++bufpos == size_t(window_size))
			bufpos = 0;
	}

	void reset()
	{
		std::fill_n(buf, window_size, 0);
		fingerprint = 0;
	}

	uint64_t value() const
	{
		return fingerprint;
	}

private:
	size_t bufpos;
	uint8_t buf[window_size];
	uint64_t fingerprint;
	uint64_t U[256];
};

}

#endif
