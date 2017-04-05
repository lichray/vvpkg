#include <vvpkg/vfile.h>

#include <sqxx/sqxx.hpp>

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

struct vfile::impl
{
	impl(char const* db, bool readonly)
	    : conn(db,
	           readonly ? sqxx::OPEN_READONLY
	                    : sqxx::OPEN_CREATE | sqxx::OPEN_READWRITE)
	{
	}

	sqxx::connection conn;
};

vfile::vfile(std::string path, char const* mode) : db_path_(std::move(path))
{
	mkdirp(db_path_.data());
	auto n = db_path_.size();
	if (not db_path_.empty() and db_path_.back() != '/')
		db_path_.push_back('/');
	db_path_.append("vvpkg.db");
	path_ = db_path_;
	path_.remove_suffix(path_.size() - n);

	bool readonly = mode == stdex::string_view("r");
	if (not readonly and mode != stdex::string_view("r+"))
		throw std::runtime_error{ R"(mode may be "r" or "r+")" };
	impl_.reset(new impl(db_path_.data(), readonly));
	if (not readonly)
	{
		impl_->conn.run(R"(
		create table if not exists cblocks (
		    id     blob primary key,
		    offset integer not null,
		    size   integer not null
		) without rowid)");
		impl_->conn.run(R"(
		create unique index if not exists size_idx
		on cblocks(offset)
		)");
	}
}

vfile::vfile(vfile&&) noexcept(
    std::is_nothrow_move_constructible<std::string>::value) = default;
vfile& vfile::operator=(vfile&&) noexcept(
    std::is_nothrow_move_assignable<std::string>::value) = default;
vfile::~vfile() = default;

void vfile::merge(std::vector<msg_digest> const& missing, bundle const& bs,
                  stdex::signature<void(int64_t, char const*, size_t)> emit)
{
	static auto feed =
	    impl_->conn.prepare("insert into cblocks values(?, ?, ?)");
	static auto getsize = impl_->conn.prepare(
	    "select offset + size from cblocks order by offset desc limit 1");

	auto it = begin(bs.blocks());
	auto ed = end(bs.blocks());

	impl_->conn.run("begin");

	getsize.run();
	auto offset = getsize ? getsize.val<int64_t>(0) : 0;
	getsize.reset();

	for (auto&& id : missing)
	{
		it = std::find_if(it, ed, [&](decltype(*it) binfo) {
			return std::get<1>(binfo) == id;
		});

		if (it == ed)
			break;

		auto blk = bs.block_at(it);
		emit(offset, blk.data(), blk.size());

		feed.bind(0, sqxx::blob(id.data(), hash::digest_size), false);
		feed.bind(1, offset);
		offset += int(blk.size());
		feed.bind(2, int(blk.size()));
		feed.run();
		feed.reset();
		feed.clear_bindings();
	}

	impl_->conn.run("commit");
}

}
