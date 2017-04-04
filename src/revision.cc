#include <sqxx/sqxx.hpp>

#include <algorithm>
#include <vvpkg/revision.h>

namespace vvpkg
{

struct revision::impl
{
	impl(char const* fn) : conn(fn, sqxx::OPEN_READWRITE) {}

	sqxx::connection conn;
};

revision::revision(std::string path, std::string const& db_path)
    : path_(std::move(path)), impl_(new impl(db_path.data()))
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

auto revision::assign_blocks(deuceclient::bundle const& bs)
    -> std::vector<deuceclient::msg_digest>
{
	static auto staging =
	    impl_->conn.prepare("insert into newrev values(?)");
	static auto missing = impl_->conn.prepare(R"(
	select distinct key
	  from newrev join cblocks using(key)
	 where cblocks.key is null
	 order by newrev.rowid
	)");
	static auto cleanup = impl_->conn.prepare("delete from newrev");

	impl_->conn.run("begin immediate");
	for (auto&& x : bs.blocks())
	{
		auto&& blockid = std::get<1>(x);
		staging.bind(0, sqxx::blob(blockid.data(),
		                           deuceclient::hash::digest_size),
		             false);
		staging.run();
	}
	impl_->conn.run("commit");

	std::vector<deuceclient::msg_digest> v;
	missing.run();
	for (auto rownum : missing)
	{
		(void)rownum;
		auto raw = missing.val<sqxx::blob>(0);
		deuceclient::msg_digest blockid;
		std::copy_n(reinterpret_cast<unsigned char const*>(raw.data),
		            raw.length, blockid.data());
		v.push_back(blockid);
	}

	cleanup.run();
	return v;
}

}
