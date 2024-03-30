/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Functions.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/29 14:07:13 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/28 19:32:30 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FUNCTIONS_HPP
# define FUNCTIONS_HPP

#include <string>
#include <sstream>
#include <vector>

#include "Types.hpp"

namespace webserv
{

template<typename T>
inline bool isUnsigned() {return false;};

template<>
inline bool isUnsigned<uint64>() {return true;};


template<typename T>
std::string to_string(const T& value)
{
    std::ostringstream os;
    os << value;
    return os.str();
}

template<>
std::string to_string(const std::vector<Byte>& value);

std::vector<Byte>   to_vector(const std::string& str);

std::vector<std::string>    splitByChars(const std::string& src, const std::string& chars);
std::string	dequote(const std::string& str);


template<typename T>
bool is(const std::string& str)
{
    if (isUnsigned<T>() && str.find('-') != std::string::npos)
        return false;

    std::istringstream	iss(str);
    T val;
    iss >> val;

    if (iss.fail())
        return false;

    while (iss.eof() == false && std::strchr(" \t\r\n\v\0", (char)iss.get()));

    return iss.eof();
}

template<typename T>
T to(const std::string& str)
{
    std::istringstream	iss(str);
    T val;

    if (is<T>(str) == false)
        throw std::runtime_error("conversion error");

    iss >> val;
    return val;
}

}

#endif // FUNCTIONS_HPP
