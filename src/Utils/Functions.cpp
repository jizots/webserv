#include <cstring>

#include "Functions.hpp"

namespace webserv
{

std::vector<Byte>   to_vector(const std::string& str)
{
	std::vector<Byte>   vec;

	vec.resize(str.size());
	std::memcpy(vec.data(), str.c_str(), str.size());
	return (vec);
}

}