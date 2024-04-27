/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:25:33 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/26 19:00:49 by tchoquet         ###   ########.fr       */
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

struct HTTPRequest : public HTTPBase
{
    inline HTTPRequest() : HTTPBase(), port(0), contentLength(0), timeout(0) {};

    std::string method;
    std::string uri;
    std::string query;
    std::string params; // ?

    std::string hostname;
    uint16 port;
    uint64 contentLength;
    uint32 timeout;
};
typedef SharedPtr<HTTPRequest> HTTPRequestPtr;

}

#endif // HTTPREQUEST_HPP
