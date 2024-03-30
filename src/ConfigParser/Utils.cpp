#include "Utils.hpp"
#include <sys/stat.h>
#include <fstream>
#include <dirent.h>
#include <cstring>

namespace webserv
{

bool	isDirectory(const std::string& path)
{
	struct stat	statBuf;
	std::string	errorMsg;

	if (::stat(path.c_str(), &statBuf) == -1)
	{
		errorMsg = path + ": " + std::strerror(errno);
		throw (ConfigException("error", 0, errorMsg, ""));
	}
	if (S_ISDIR(statBuf.st_mode))
		return (true);
	return (false);
}

DIR*	openDirectory(const std::string& path)
{
	DIR*			dir;
	std::string		errorMsg;

	dir = ::opendir(path.c_str());
	if (dir == NULL)
	{
		errorMsg = path + ": " + ::strerror(errno);
		throw (ConfigException("error", 0, errorMsg, ""));
	}
	return (dir);
}

bool	isFileAccessible(const std::string& filePath)
{
	std::ifstream	ifs(filePath.c_str());
	std::string		errorMsg;

	if (!ifs.is_open())
	{
		errorMsg = filePath + ": " + ::strerror(errno);
		throw (ConfigException("error", 0, errorMsg, ""));
	}
	if (isDirectory(filePath))
	{
		errorMsg = std::string(filePath) + ": is directory";
		ifs.close();
		throw (ConfigException("error", 0, errorMsg, ""));
	}
	ifs.close();
	return (true);
}

bool	isNumericLiteral(const std::string& str)
{
	size_t	i = 0;
	bool	hasDigits = false;

	for ( ;i < str.size() && std::isspace(static_cast<int>(str[i])); ++i)
		i++;
	if (i < str.size() && (str[i] == '+' || str[i] == '-'))
		i++;
	for ( ;i < str.size() && std::isdigit(static_cast<int>(str[i])); ++i)
		hasDigits = true;
	return (hasDigits && i == str.size());
}

bool	isPositiveNum(const std::string& str)
{
	size_t	i = 0;
	bool	hasDigits = false;

	for ( ;i < str.size() && std::isspace(static_cast<int>(str[i])); ++i)
		i++;
	if (i < str.size() && (str[i] == '+'))
		i++;
	for ( ;i < str.size() && std::isdigit(static_cast<int>(str[i])); ++i)
		hasDigits = true;
	return (hasDigits && i == str.size());
}

bool	isInt(const std::string& literal)
{
	std::istringstream	iss(literal);
	int					val;
	iss >> val;

	return (iss && isNumericLiteral(literal));
}

bool	isSizet(const std::string& literal)
{
	std::istringstream	iss(literal);
	size_t				val;
	iss >> val;

	return (iss && isPositiveNum(literal));
}

bool	isCharInSet(const char c, const char* set)
{
	if (set == NULL)
		return (false);
	return (std::strchr(set, c));
};

}
