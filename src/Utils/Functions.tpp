/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Functions.tpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/01 12:51:46 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/08 11:27:38 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FUNCTIONS_TPP
#define FUNCTIONS_TPP

#include "Utils/Functions.hpp"

#include <set>
#include <stdexcept>
#include <cstring>

namespace webserv
{

template<typename T>
std::string to_string(const T& value)
{
    std::ostringstream os;
    os << value;
    return os.str();
}

template<typename T>
bool is(const std::string& str)
{
    if (isUnsigned<T>() && str.find('-') != std::string::npos)
        return false;

    std::istringstream iss(str);
    T val;
    iss >> val;

    if (iss.fail())
        return false;

    while (iss.eof() == false && std::strchr(" \t\r\n\v\0", (char)iss.get()))
        ;

    return iss.eof();
}

template<typename T>
T to(const std::string& str)
{
    std::istringstream iss(str);
    T val;

    if (is<T>(str) == false)
        throw std::runtime_error("conversion error");

    iss >> val;
    return val;
}

template<typename T>
bool hasDuplicate(const std::vector<T>& vec)
{
    std::set<T> seen;

    for (typename std::vector<T>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (!seen.insert(*it).second)
            return (true);
    }
    return (false);
}

} // namespace webserv

#endif // FUNCTIONS_TPP
