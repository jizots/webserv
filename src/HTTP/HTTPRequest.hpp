/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:25:33 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/20 13:33:48 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include "HTTP/HTTPBase.hpp"

#include <string>
#include <map>
#include <vector>
#include <iostream>

#include "Utils/Types.hpp"
#include "Utils/SharedPtr.hpp"

namespace webserv
{

struct HTTPHostValue
{
    std::string hostname;
    uint16 port;
};

struct HTTPFieldValue
{
    std::string valName;
    std::map<std::string, std::string> parameters;
};

struct HTTPRequest : public HTTPBase
{
    inline HTTPRequest() : HTTPBase(), contentLength(0), isChunk(false){};

    std::string method;
    std::string uri;
    std::string query;
    std::string params; // ?
    HTTPFieldValue httpFieldValue;
    std::vector<HTTPFieldValue> httpFieldValues;

    HTTPHostValue host;
    uint64 contentLength;
    bool isChunk;
};
typedef SharedPtr<HTTPRequest> HTTPRequestPtr;

}

#endif // HTTPREQUEST_HPP
