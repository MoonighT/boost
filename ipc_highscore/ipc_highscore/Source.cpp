#include <boost/filesystem.hpp> 
#include <boost\bind.hpp>
#include <iostream> 
#include <vector>
#include <iterator>
#include <algorithm>
using namespace boost::filesystem;


void printDirectory(path p, int level)
{
	if (is_regular_file(p))
	{
		for (int i = 0; i < level; ++i)
			printf("  ");
		std::cout << p.filename() << '\n';
	}
	else if (is_directory(p))
	{
		for (int i = 0; i < level; ++i)
			printf("  ");
		std::cout << p.filename() << "\\ \n";
		typedef std::vector< path > vec;
		vec v;
		std::copy(directory_iterator(p), directory_iterator(), std::back_inserter(v));

		std::sort(v.begin(), v.end());
		std::for_each(v.begin(), v.end(), boost::bind(printDirectory, _1, level + 1));
	}
	else
	{
		std::cout << p << " exists, but is neither a regular file nor a directory\n";
	}
}

int main(int argc, char* argv[])
{
	path p(L"D:\\software\\Beyond Compare 3");

	try
	{
		if (exists(p))    // does p actually exist?
		{
			printDirectory(p, 0);
		}
		else
			std::cout << p << " does not exist\n";
	}

	catch (const filesystem_error& ex)
	{
		std::cout << ex.what() << '\n';
	}

	return 0;
}