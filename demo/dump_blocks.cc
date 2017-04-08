#include <vvpkg/vvpkg.h>
#include <vvpkg/fd_funcs.h>

#include "demo_helpers.h"

void dump_blocks(char const* filename);

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cerr << "usage: ./dump_blocks <PATH>\n";
		exit(2);
	}

	try
	{
		dump_blocks(argv[1]);
	}
	catch (std::exception& e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl;
		exit(1);
	}
}

void dump_blocks(char const* filename)
{
	int fd = vvpkg::xopen_for_read(filename);
	defer(vvpkg::xclose(fd));

	vvpkg::managed_bundle<rax::rabin_boundary> bs(10 * 1024 * 1024);

	int64_t file_size = 0;
	bool bundle_is_full;

	std::cout.sync_with_stdio(false);
	std::cout << '[';
	bool first_item = true;

	do
	{
		bundle_is_full = bs.consume(vvpkg::from_descriptor(fd));

		if (bs.empty())
			break;

		int64_t offset = file_size;

		for (auto&& t : bs.blocks())
		{
			size_t end_of_block;
			vvpkg::msg_digest blockid;
			std::tie(end_of_block, blockid) = t;

			if (not first_item)
				std::cout << ',';

			first_item = false;
			char buf[2 * vvpkg::hash::digest_size];
			hashlib::detail::hexlify_to(blockid, buf);
			std::cout << "[\""
			          << stdex::string_view(buf, sizeof(buf))
			          << "\"," << offset << ']';

			offset = file_size + int64_t(end_of_block);
		}

		file_size += int64_t(bs.size());
		bs.clear();

	} while (bundle_is_full);

	std::cout << "]\n";
}
