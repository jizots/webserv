/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ekamada <ekamada@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:25:33 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/17 15:50:50 by ekamada          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include "HTTP/HTTPBase.hpp"

namespace webserv
{

struct HTTPRequest : public HTTPBase
{
    inline HTTPRequest()
        : HTTPBase(),
          contentLength(0),
          isBadRequest(false) {};

    inline HTTPRequest(const HTTPRequest& cp, const std::string& method, const std::string& uri)
        : HTTPBase(cp),
          method(method),
          uri(uri),
          host(cp.host),
          contentLength(cp.contentLength),
          isBadRequest(cp.isBadRequest) {};

    inline HTTPRequest(const HTTPRequest& cp, bool isBadRequest)
        : HTTPBase(cp),
          method(cp.method),
          uri(cp.uri),
          host(cp.host),
          contentLength(cp.contentLength),
          isBadRequest(isBadRequest) {};

    std::string method;
    std::string uri;
    std::string query;
    std::string params;
    std::string host;
    uint64 contentLength;

    bool isBadRequest;
};
typedef SharedPtr<HTTPRequest> HTTPRequestPtr;

}

#endif // HTTPREQUEST_HPP
