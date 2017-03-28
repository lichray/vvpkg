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

#include <deuceclient/rabin_fingerprint.h>

#define MSB64 uint64_t(0x8000000000000000)

#if defined(_MSC_VER)
#include <intrin.h>

__forceinline
int __builtin_clzll(unsigned long long mask)
{
	unsigned long where;

#if defined(_WIN64)
	if (_BitScanReverse64(&where, mask))
		return static_cast<int>(63 - where);
#elif defined(_WIN32)
	if (_BitScanReverse(&where, static_cast<unsigned long>(mask >> 32)))
		return static_cast<int>(63 - (where + 32));
	if (_BitScanReverse(&where, static_cast<unsigned long>(mask)))
		return static_cast<int>(63 - where);
#else
#error "Implementation of __builtin_clzll required"
#endif
	return 64;  // UB
}

#endif

inline
int fls64(uint64_t x)
{
	return sizeof(x) * 8 - __builtin_clzll(x);
}

static
uint64_t polymod(uint64_t nh, uint64_t nl, uint64_t d)
{
	int k = fls64(d) - 1;
	d <<= 63 - k;

	if (nh)
	{
		if (nh & MSB64)
			nh ^= d;

		for (int i = 62; i >= 0; i--)
		{
			if (nh & uint64_t(1) << i)
			{
				nh ^= d >> (63 - i);
				nl ^= d << (i + 1);
			}
		}
	}

	for (int i = 63; i >= k; i--)
		if (nl & uint64_t(1) << i)
			nl ^= d >> (63 - i);

	return nl;
}

static
void polymult(uint64_t *php, uint64_t *plp, uint64_t x, uint64_t y)
{
	uint64_t ph = 0, pl = 0;

	if (x & 1)
		pl = y;

	for (int i = 1; i < 64; i++)
	{
		if (x & uint64_t(1) << i)
		{
			ph ^= y >> (64 - i);
			pl ^= y << i;
		}

		if (php)
			*php = ph;
		if (plp)
			*plp = pl;
	}
}

namespace rax
{

uint64_t _polymmult(uint64_t x, uint64_t y, uint64_t d)
{
	uint64_t h, l;
	polymult(&h, &l, x, y);

	return polymod(h, l, d);
}

rabin_polynomial::rabin_polynomial(uint64_t poly) :
	poly(poly)
{
	calculate_lookup_table();
}

void rabin_polynomial::calculate_lookup_table()
{
	int xshift = fls64(poly) - 1;
	shift = xshift - 8;
	auto T1 = polymod(0, uint64_t(1) << xshift, poly);

	for (uint64_t j = 0; j < 256; j++)
		T[j] = _polymmult(j, T1, poly) | (j << xshift);
}

}
