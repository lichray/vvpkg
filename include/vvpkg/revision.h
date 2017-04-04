#pragma once

#include <deuceclient/bundle.h>
#include <vector>

namespace vvpkg
{

namespace deuceclient = rax::deuceclient;

struct revision
{
	explicit revision(std::string path, std::string const& db_path);

	revision(revision&&) noexcept(
	    std::is_nothrow_move_constructible<std::string>::value);
	revision& operator=(revision&&) noexcept(
	    std::is_nothrow_move_assignable<std::string>::value);
	~revision();

	stdex::string_view id() const { return path_; }

	auto assign_blocks(deuceclient::bundle const& bs)
	    -> std::vector<deuceclient::msg_digest>;
	void finalize_file(int64_t len);

	friend bool operator==(revision const& a, revision const& b) noexcept
	{
		return a.path_ == b.path_;
	}

	friend bool operator!=(revision const& a, revision const& b) noexcept
	{
		return !(a == b);
	}

private:
	struct impl;

	std::string path_;
	std::unique_ptr<impl> impl_;
};

}
