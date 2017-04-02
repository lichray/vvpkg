#include <sqxx/sqxx.hpp>

#include <vvpkg/vfile.h>

#if defined(_WIN32)
#include <direct.h>
#else
#include <sys/stat.h>
#define _mkdir(fn) ::mkdir(fn, 0755)
#endif
#include <system_error>

static void mkdirp(char const* fn)
{
	errno = 0;
	_mkdir(fn);
	if (errno == EEXIST)
		errno = 0;
	else if (errno != 0)
		throw std::system_error(errno, std::system_category());
}

namespace vvpkg
{

vfile::vfile(std::string path) : db_path_(std::move(path))
{
	mkdirp(db_path_.data());
	auto n = db_path_.size();
	if (not db_path_.empty() and db_path_.back() != '/')
		db_path_.push_back('/');
	db_path_.append("vvpkg.db");
	path_ = db_path_;
	path_.remove_suffix(path_.size() - n);

	sqxx::connection conn(db_path_,
	                      sqxx::OPEN_CREATE | sqxx::OPEN_READWRITE);
	conn.run(R"(
	create table if not exists cblocks (
	    id     blob primary key,
	    offset integer not null,
	    size   integer not null
	) without rowid)");
}

}
