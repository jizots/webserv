/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Functions.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sotanaka <sotanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/29 14:07:13 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/18 14:11:24 by sotanaka         ###   ########.fr       */
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
std::string to_string(const T& value)
{
    std::ostringstream os;
    os << value;
    return os.str();
}
std::vector<Byte>   to_vector(const std::string& str);

template<typename T>
bool is(const std::string& str)
{
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