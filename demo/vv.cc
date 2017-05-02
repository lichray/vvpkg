#include <vvpkg/vvpkg.h>
#include <vvpkg/fd_funcs.h>

#include "demo_helpers.h"

void commit(char const* rev, char const* fn);
void checkout(char const* rev, char const* fn);

using namespace stdex::literals;

int main(int argc, char* argv[])
{
	if (argc < 4)
	{
	err:
		std::cerr << "usage: ./vv commit|checkout <REV> <PATH>\n";
		exit(2);
	}

	try
	{
		if (argv[1] == "commit"_sv)
			commit(argv[2], argv[3]);
		else if (argv[1] == "checkout"_sv)
			checkout(argv[2], argv[3]);
		else
			goto err;
	}
	catch (std::exception& e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
}

void commit(char const* rev, char const* fn)
{
	auto repo = vvpkg::repository(".", "r+");
	vvpkg::managed_bundle<vvpkg::rabin_boundary> bs(10 * 1024 * 1024);
	int fd = (fn == "-"_sv ? 0 : vvpkg::xopen_for_read(fn));
	defer(vvpkg::xclose(fd));

	repo.commit(rev, bs, vvpkg::from_descriptor(fd));
}

void checkout(char const* rev, char const* fn)
{
	auto repo = vvpkg::repository(".", "r");
	int fd = (fn == "-"_sv ? 1 : vvpkg::xopen_for_write(fn));
	defer(vvpkg::xclose(fd));

	repo.checkout(rev, vvpkg::to_descriptor(fd));
}
