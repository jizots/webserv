/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPBase.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/18 18:47:29 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/06 16:33:50 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPBASE_HPP
# define HTTPBASE_HPP

#include <map>
#include <vector>
#include <string>

#include "Utils/Utils.hpp"

namespace webserv
{

struct HTTPBase
{
    HTTPBase(uint8 versionMajor = 1, uint8 versionMinor = 1)
        : httpVersionMajor(versionMajor), httpVersionMinor(versionMinor) {}

    uint8 httpVersionMajor;
    uint8 httpVersionMinor;
    std::map<std::string, std::string> headers;
    std::vector<Byte> body;

    inline std::string httpVersionStr() const { return "HTTP/" + to_string((int)httpVersionMajor) + '.' + to_string((int)httpVersionMinor); }
};

}

#endif // HTTPBASE_HPP
