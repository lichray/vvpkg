#pragma once

#include <deuceclient/rabin_fingerprint.h>

namespace vvpkg
{

struct rabin_boundary
{
	void process_byte(char byte)
	{
		process_byte(static_cast<unsigned char>(byte));
	}

	void process_byte(unsigned char byte) { rabinfp_.process_byte(byte); }
	bool reached_boundary(size_t size_read);
	void reset() { rabinfp_.reset(); }

private:
	static constexpr uint64_t fingerprint_pt = 0xbfe6b8a5bf378d83;
	static constexpr uint64_t break_value = 7;
	static constexpr size_t avg_ = 16384;
	static constexpr size_t min_ = 12288;
	static constexpr size_t max_ = 98304;

	rax::rabin_fingerprint<128> rabinfp_{ fingerprint_pt };
};

inline bool rabin_boundary::reached_boundary(size_t size_read)
{
	if (size_read == max_)
		return true;
	else if (size_read >= min_ and
	         (rabinfp_.value() % avg_ == break_value))
		return true;
	else
		return false;
}

}
