#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include <sstream>
# include <set>
# include <vector>

# include "ConfigParser/ConfigParser.hpp"

bool	isDirectory(const std::string& path);
bool	isFileAccessible(const std::string& path);
bool	isNumericLiteral(const std::string& str);
bool	isInt(const std::string& literal);
bool	isSizet(const std::string& literal);
bool	isCharInSet(const char c, const char* set);

template <typename T>
T	convertStrToType(const std::string& str, bool (*func)(const std::string&))
{
	std::istringstream	iss(str);
	T					val;
	std::string			errorMsg;

	iss >> val;
	if (!iss || !func(str))
	{
		errorMsg = "Invalid number: " + str;
		throw (ConfigException("error", 0, errorMsg, ""));
	}
	return (val);
};

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

#endif