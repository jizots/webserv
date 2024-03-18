/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestLineParser.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:27:25 by ekamada           #+#    #+#             */
/*   Updated: 2024/03/17 18:07:09 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTLINEPARSER_HPP
#define REQUESTLINEPARSER_HPP

#include <map>
#include <vector>
#include <string>

#include "Utils/Utils.hpp"
#include "ConfigParser/Utils.hpp"
#include "HTTPUtils.hpp"
#include "HTTP/HTTPRequest.hpp"
namespace webserv
{


class RequestLineParser
{
private:
    enum status{
        _requestMethod,
        _slash,
        _uri,
        _params,
        _query,
        _HTTP,
        _verMajor,
        _dot,
        _verMinor,
        _endRequest,
        _parseComplete,
        _badRequest
    };
public:
    RequestLineParser(HTTPRequest &request);

    void parse(Byte c);
    inline bool isComplete() {return (m_status == _parseComplete);};

private:
    void checkCRLF(Byte c, int successStatus);
    void decodeHex(Byte c, std::string& dst);

    HTTPRequest &m_request;

    int m_status;
    std::string m_hex;
    bool m_foundCR;
    std::string m_protocol;
};

}


#endif
