#pragma once

#include "revision.h"

namespace vvpkg
{

struct vfile
{
	explicit vfile(std::string path);

	stdex::string_view id() const { return path_; }

	revision new_revision(std::string commitid);

	friend bool operator==(vfile const& a, vfile const& b) noexcept
	{
		return a.path_ == b.path_;
	}

	friend bool operator!=(vfile const& a, vfile const& b) noexcept
	{
		return !(a == b);
	}

private:
	stdex::string_view path_;
	std::string db_path_;
};

inline revision vfile::new_revision(std::string commitid)
{
	auto prefix = stdex::string_view(db_path_).substr(0, path_.size() + 1);
	commitid.insert(0, prefix.data(), prefix.size());
	return revision(std::move(commitid), db_path_);
}

}
