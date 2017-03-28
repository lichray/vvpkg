#include <fcntl.h>
#if defined(WIN32)
#include <sys/stat.h>
#include <io.h>
#include <share.h>
#else
#include <unistd.h>
#endif

#include "demo_helpers.h"

using namespace rax;

void dump_blocks(char const* filename);

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cerr << "usage: ./dump_blocks <PATH>\n";
		exit(1);
	}

	try
	{
		dump_blocks(argv[1]);
	}
	catch (std::exception& e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl;
	}
}

void dump_blocks(char const* filename)
{
#if defined(WIN32)
	int fd;
	_sopen_s(&fd, filename, _O_RDONLY | _O_BINARY , _SH_DENYWR, _S_IREAD);
#else
	auto fd = open(filename, O_RDONLY);
#endif

	if (fd == -1)
		THROW_ERRNO();

#if defined(WIN32)
	defer(_close(fd));
#else
	defer(close(fd));
#endif

	deuceclient::managed_bundle<rabin_boundary> bs(10 * 1024 * 1024);
	bs.boundary().set_limits(14843, 17432, 90406);

	int64_t file_size = 0;
	bool bundle_is_full;

	std::cout.sync_with_stdio(false);
	std::cout << '[';
	bool first_item = true;

	do
	{
		bundle_is_full = bs.consume(
		    [=](char* p, size_t sz)
		    {
#if defined(WIN32)
			return _read(fd, p, unsigned(sz));
#else
			return read(fd, p, sz);
#endif
		    });

		if (bs.empty())
			break;

		int64_t offset = file_size;

#if !(defined(_MSC_VER) && _MSC_VER < 1800)
		for (auto&& t : bs.blocks())
#else
		for each (auto&& t in bs.blocks())
#endif
		{
			size_t end_of_block;
			deuceclient::sha1_digest blockid;
			std::tie(end_of_block, blockid) = t;

			if (not first_item)
				std::cout << ',';

			first_item = false;
			char buf[40];
			hashlib::detail::hexlify_to(blockid, buf);
			std::cout << "[\""
			    << stdex::string_view(buf, sizeof(buf))
			    << "\"," << offset << ']';

			offset = file_size + end_of_block;
		}

		file_size += bs.size();
		bs.clear();

	} while (bundle_is_full);

	std::cout << "]\n";
}
