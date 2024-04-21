/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPBase.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/18 18:47:29 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/08 17:41:59 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPBASE_HPP
# define HTTPBASE_HPP

#include <map>
#include <string>

#include "Utils/Utils.hpp"

namespace webserv
{

struct HTTPBase
{
    HTTPBase(uint8 verMajor = 1, uint8 verMinor = 1) : verMajor(verMajor), verMinor(verMinor) {}

    uint8 verMajor, verMinor;
    std::map<std::string, std::string> headers;
    std::vector<Byte> body;

    inline std::string httpVersionStr() const { return "HTTP/" + to_string((int)verMajor) + '.' + to_string((int)verMinor); }
};

}

#endif // HTTPBASE_HPP
