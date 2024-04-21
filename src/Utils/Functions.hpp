/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Functions.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/29 14:07:13 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/08 17:29:43 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include <string>
#include <sstream>
#include <vector>

#include "Types.hpp"

namespace webserv
{
    
std::vector<Byte> to_vector(const std::string& str);

std::vector<std::string> splitByChars(const std::string& src, const std::string& chars);

std::string dequote(const std::string& str);

bool isDirectory(const std::string& path);

bool isFileAccessible(const std::string& path);

bool isNumericLiteral(const std::string& str);

bool isInt(const std::string& literal);

bool isSizet(const std::string& literal);

std::string trimCharacters(const std::string& str, const std::string& charSet);

bool    hasCommonCharacter(const std::string& s1, const std::string& s2);
std::string	stringToLower(const std::string& str);
bool	compStringCaseInsensitive(const std::string& s1, const std::string& s2);

template<typename T>
inline bool isUnsigned() { return false; };

template<>
inline bool isUnsigned<uint64>() { return true; };

template<typename T>
std::string to_string(const T& value);

template<>
std::string to_string(const std::vector<Byte>& value);

template<typename T>
bool is(const std::string& str);

template<typename T>
T to(const std::string& str);

template<typename T>
bool hasDuplicate(const std::vector<T>& vec);


} // namespace webserv

#ifndef FUNCTIONS_TPP
    #include "Utils/Functions.tpp"
#endif // FUNCTIONS_TPP

#endif // FUNCTIONS_HPP
