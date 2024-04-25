/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Functions.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sotanaka <sotanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/01 14:59:13 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/21 19:24:59 by sotanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Functions.hpp"

#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>

namespace webserv
{

std::vector<Byte> to_vector(const std::string& str)
{
    std::vector<Byte> vec;

    vec.resize(str.size());
    std::memcpy(vec.data(), str.c_str(), str.size());
    return (vec);
}

template<>
std::string to_string(const std::vector<Byte>& value)
{
    return (std::string(value.begin(), value.end()));
}

void controlQuoteFlag(bool& hasDquote, const std::string& src, const size_t i)
{
    if (hasDquote)
    {
        if (2 < i && src[i - 1] != '\\' && src[i -2] != '\\')
            hasDquote = false;
    }
    else
        hasDquote = true;
}

std::vector<std::string> splitQuotedStringByChars(const std::string& src, const std::string& chars)
{
    std::vector<std::string> splitedStr;
    size_t i = 0;
    size_t wc = 0;
    bool hasDelimiter = false;
    bool hasDquote = false;

    for (; i < src.size(); ++i)
    {
        if (src[i] == '"')
            controlQuoteFlag(hasDquote, src, i);
        if (hasDquote == false && std::strchr(chars.c_str(), static_cast<int>(src[i])))
        {
            if (wc)
                hasDelimiter = true;
        }
        else
            ++wc;
        if (hasDelimiter && wc)
        {
            splitedStr.push_back(std::string(src, i - wc, wc));
            wc = 0;
            hasDelimiter = false;
        }
    }
    if (wc)
        splitedStr.push_back(std::string(src, i - wc, wc));
    return (splitedStr);
};

std::vector<std::string> splitByChars(const std::string& src, const std::string& chars)
{
    std::vector<std::string> splitedStr;
    size_t i = 0;
    size_t wc = 0;
    bool hasDelimiter = false;

    for (; i < src.size(); ++i)
    {
        if (std::strchr(chars.c_str(), static_cast<int>(src[i])))
        {
            if (wc)
                hasDelimiter = true;
        }
        else
            ++wc;
        if (hasDelimiter && wc)
        {
            splitedStr.push_back(std::string(src, i - wc, wc));
            wc = 0;
            hasDelimiter = false;
        }
    }
    if (wc)
        splitedStr.push_back(std::string(src, i - wc, wc));
    return (splitedStr);
};

std::string dequote(const std::string& str)
{
    if (str.size() == 1)
        return (str);
    if (str[0] == '"' && str[str.size() - 1] == '"')
        return (str.substr(1, str.size() - 2));
    return (str);
}

bool isDirectory(const std::string& path)
{
    struct stat statBuf;
    std::string errorMsg;

    if (::stat(path.c_str(), &statBuf) == -1)
    {
        errorMsg = path + ": " + std::strerror(errno);
        // throw(ConfigException("error", 0, errorMsg, ""));
        throw std::runtime_error(errorMsg);
    }
    if (S_ISDIR(statBuf.st_mode))
        return (true);
    return (false);
}

DIR* openDirectory(const std::string& path)
{
    DIR* dir;
    std::string errorMsg;

    dir = ::opendir(path.c_str());
    if (dir == NULL)
    {
        errorMsg = path + ": " + ::strerror(errno);
        // throw(ConfigException("error", 0, errorMsg, ""));
        throw std::runtime_error(errorMsg);
    }
    return (dir);
}

bool isFileAccessible(const std::string& filePath)
{
    std::ifstream ifs(filePath.c_str());
    std::string errorMsg;

    if (!ifs.is_open())
    {
        errorMsg = filePath + ": " + ::strerror(errno);
        // throw(ConfigException("error", 0, errorMsg, ""));
        throw std::runtime_error(errorMsg);
    }
    if (isDirectory(filePath))
    {
        errorMsg = std::string(filePath) + ": is directory";
        ifs.close();
        // throw(ConfigException("error", 0, errorMsg, ""));
        throw std::runtime_error(errorMsg);
    }
    ifs.close();
    return (true);
}

bool isNumericLiteral(const std::string& str)
{
    size_t i = 0;
    bool hasDigits = false;

    for (; i < str.size() && std::isspace(static_cast<int>(str[i])); ++i)
        i++;
    if (i < str.size() && (str[i] == '+' || str[i] == '-'))
        i++;
    for (; i < str.size() && std::isdigit(static_cast<int>(str[i])); ++i)
        hasDigits = true;
    return (hasDigits && i == str.size());
}

bool isPositiveNum(const std::string& str)
{
    size_t i = 0;
    bool hasDigits = false;

    for (; i < str.size() && std::isspace(static_cast<int>(str[i])); ++i)
        i++;
    if (i < str.size() && (str[i] == '+'))
        i++;
    for (; i < str.size() && std::isdigit(static_cast<int>(str[i])); ++i)
        hasDigits = true;
    return (hasDigits && i == str.size());
}

bool isInt(const std::string& literal)
{
    std::istringstream iss(literal);
    int val;
    iss >> val;

    return (iss && isNumericLiteral(literal));
}

bool isSizet(const std::string& literal)
{
    std::istringstream iss(literal);
    size_t val;
    iss >> val;

    return (iss && isPositiveNum(literal));
}

std::string trimCharacters(const std::string& str, const std::string& charSet)
{
    const size_t start = str.find_first_not_of(charSet);

    if (start == std::string::npos)
        return "";
    
    const size_t end = str.find_last_not_of(charSet);
    return str.substr(start, end - start + 1);
}

bool    hasCommonCharacter(const std::string& s1, const std::string& s2)
{
	return (s1.find_first_of(s2) != std::string::npos);
};

std::string	stringToLower(const std::string& str)
{
	std::string	result(str);

	for (std::string::size_type i = 0; i < str.size(); ++i)
		result[i] = std::tolower(static_cast<int>(result[i]));
	return (result);
};

bool	compStringCaseInsensitive(const std::string& s1, const std::string& s2)
{
	if (stringToLower(s1) == stringToLower(s2))
		return (true);
	return (false);
};

} // namespace webserv
