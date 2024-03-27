/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:25:33 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/23 17:29:10 by tchoquet         ###   ########.fr       */
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

    std::string method;
    std::string uri;
    std::string query;
    std::string params; // ?
    std::string host;
    uint64 contentLength;

    bool isBadRequest;
};
typedef SharedPtr<HTTPRequest> HTTPRequestPtr;

}

#endif // HTTPREQUEST_HPP
