#include <sqxx/sqxx.hpp>

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
}

revision::revision(revision&&) noexcept(
    std::is_nothrow_move_constructible<std::string>::value) = default;
revision& revision::operator=(revision&&) noexcept(
    std::is_nothrow_move_assignable<std::string>::value) = default;
revision::~revision() = default;

}
