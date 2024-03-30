#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <sstream>
# include <set>
# include <vector>

# include "ConfigParser/ConfigParser.hpp"

namespace webserv
{

bool	isDirectory(const std::string& path);
bool	isFileAccessible(const std::string& path);
bool	isNumericLiteral(const std::string& str);
bool	isInt(const std::string& literal);
bool	isSizet(const std::string& literal);
bool	isCharInSet(const char c, const char* set);

template <typename T>
bool	hasDuplicate(const std::vector<T>& vec)
{
	std::set<T>	seen;
	
	for (typename std::vector<T>::const_iterator it = vec.begin(); it != vec.end(); ++it)
	{
		if (!seen.insert(*it).second)
			return (true);
	}
	return (false);
}

}

#endif