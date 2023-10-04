#include "File.h"
#include <fstream>

std::vector<char> readfile(const std::string& fname)
{
	std::ifstream file(fname, std::ios::ate | std::ios::binary);
	assert(file.is_open());
	size_t fsize = file.tellg();

	std::vector<char> buffer(fsize);
	file.seekg(0);
	file.read(buffer.data(), fsize);
	file.close();

	return buffer;
}