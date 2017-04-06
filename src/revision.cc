#include <vvpkg/revision.h>
#include <vvpkg/c_file_funcs.h>
#include "manifest_creater.h"

#include <sqxx/sqxx.hpp>
#include <rapidjson/filewritestream.h>

#include <algorithm>
#include <future>

namespace vvpkg
{

struct revision::impl
{
	impl(char const* db, char const* fn)
	    : conn(db, sqxx::OPEN_READWRITE),
	      manifest(xfopen(fn, "wb"), buf, sizeof(buf))
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

	impl_->conn.run("begin");
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
		auto p = reinterpret_cast<unsigned char const*>(raw.data);
		if (raw.length != int(hash::digest_size))
			throw std::runtime_error{ "malicious hash size" };
#if !defined(_MSC_VER)
		std::copy_n(p, raw.length, blockid.data());
#else
		std::copy_n(p, raw.length,
		            stdext::make_checked_array_iterator(
		                blockid.data(), blockid.size()));
#endif
		v.push_back(blockid);
	}
	missing.reset();

	cleanup.run();
	cleanup.reset();
	assigning.wait();

	return v;
}

void revision::finalize_file(int64_t)
{
	impl_->manifest.close();
}

}
