/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPUtils.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: emukamada <emukamada@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:52:09 by ekamada           #+#    #+#             */
/*   Updated: 2024/03/11 22:22:55 by emukamada        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/HTTPUtils.hpp"


namespace webserv
{

bool isUnReserved(Byte c) {
	std::string other = "!*`(),$-_.";
	return (std::isalpha(c) || std::isdigit(c) || other.find(c) != std::string::npos);
}
bool isReserved(Byte c) {
	std::string pchar = ";/?:@&=+";
	return (isUnReserved(c) || pchar.find(c) != std::string::npos );
}
bool isPchar(Byte c) {
	std::string pchar = ":@&=+/";
	return (isUnReserved(c) || pchar.find(c) != std::string::npos );
}

bool isToken(Byte c) {
	std::string token = "!#$%&'*+-.^_`|~";
	return (std::isalpha(c) || std::isdigit(c) || token.find(c) != std::string::npos);
}

bool isPrintableAscii(Byte c) { return (c >= ' ' && c <= '~'); }

std::string trimOptionalSpace(const std::string& str) {
	const std::string space = " \t";
	const size_t start = str.find_first_not_of(space);
	if (start == std::string::npos) return "";
	const size_t end = str.find_last_not_of(space);
	return str.substr(start, end - start + 1);
}


}
