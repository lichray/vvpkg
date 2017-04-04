#include <vvpkg/revision.h>
#include "manifest_creater.h"

#include <sqxx/sqxx.hpp>
#include <rapidjson/filewritestream.h>

#include <algorithm>
#include <future>

namespace vvpkg
{

static auto fopen_for_write(char const* fn)
{
#if !defined(_WIN32)
	auto fp = ::fopen(fn, "wb");
#else
	FILE* fp;
	fopen_s(&fp, fn, "wb");
#endif
	if (fp == nullptr)
		throw std::system_error(errno, std::system_category());

	return fp;
}

struct revision::impl
{
	impl(char const* db, char const* fn)
	    : conn(db, sqxx::OPEN_READWRITE),
	      manifest(fopen_for_write(fn), buf, sizeof(buf))
	{
	}

	sqxx::connection conn;
	manifest_creater<rapidjson::FileWriteStream> manifest;
	char buf[4096];
};

revision::revision(std::string path, std::string const& db_path)
    : path_(std::move(path)),
      impl_(new impl(db_path.data(), (path_ + ".json").data()))
{
	impl_->conn.run(R"(
	create temporary table if not exists newrev (
	    id blob not null,
	    foreign key(id) references cblocks(id)
	))");
}

revision::revision(revision&&) noexcept(
    std::is_nothrow_move_constructible<std::string>::value) = default;
revision& revision::operator=(revision&&) noexcept(
    std::is_nothrow_move_assignable<std::string>::value) = default;
revision::~revision() = default;

auto revision::assign_blocks(bundle const& bs) -> std::vector<msg_digest>
{
	static auto staging =
	    impl_->conn.prepare("insert into newrev values(?)");
	static auto missing = impl_->conn.prepare(R"(
	select distinct id
	  from newrev left join cblocks using(id)
	 where cblocks.id is null
	 order by newrev.rowid
	)");
	static auto cleanup = impl_->conn.prepare("delete from newrev");

	auto assigning = std::async(std::launch::async, [&] {
		for (auto&& x : bs.blocks())
			impl_->manifest.append(std::get<1>(x));
	});

	impl_->conn.run("begin immediate");
	for (auto&& x : bs.blocks())
	{
		auto&& blockid = std::get<1>(x);
		staging.bind(0, sqxx::blob(blockid.data(), hash::digest_size),
		             false);
		staging.run();
		staging.reset();
		staging.clear_bindings();
	}
	impl_->conn.run("commit");

	std::vector<msg_digest> v;
	missing.run();
	for (auto rownum : missing)
	{
		(void)rownum;
		auto raw = missing.val<sqxx::blob>(0);
		msg_digest blockid;
		std::copy_n(reinterpret_cast<unsigned char const*>(raw.data),
		            raw.length, blockid.data());
		v.push_back(blockid);
	}

	cleanup.run();
	assigning.wait();

	return v;
}

void revision::finalize_file(int64_t)
{
	impl_->manifest.close();
}

}
