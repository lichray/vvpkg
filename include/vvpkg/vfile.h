#pragma once

#include "revision.h"

#include <stdex/functional.h>

namespace vvpkg
{

struct vfile
{
	explicit vfile(std::string path, char const* mode = "r+");

	vfile(vfile&&) noexcept(
	    std::is_nothrow_move_constructible<std::string>::value);
	vfile& operator=(vfile&&) noexcept(
	    std::is_nothrow_move_assignable<std::string>::value);
	~vfile();

	stdex::string_view id() const { return path_; }

	revision new_revision(std::string commitid);
	void merge(std::vector<msg_digest> const& missing, bundle const& bs,
	           stdex::signature<void(int64_t, char const*, size_t)> emit);
	auto list(std::string commitid)
	    -> std::function<std::pair<int64_t, int64_t>()>;

	friend bool operator==(vfile const& a, vfile const& b) noexcept
	{
		return a.path_ == b.path_;
	}

	friend bool operator!=(vfile const& a, vfile const& b) noexcept
	{
		return !(a == b);
	}

private:
	struct impl;

	stdex::string_view path_;
	std::string db_path_;
	std::unique_ptr<impl> impl_;
};

inline revision vfile::new_revision(std::string commitid)
{
	auto prefix = stdex::string_view(db_path_).substr(0, path_.size() + 1);
	commitid.insert(0, prefix.data(), prefix.size());
	return revision(std::move(commitid), db_path_);
}

}
