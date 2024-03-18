/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/15 15:53:02 by tchoquet          #+#    #+#             */
/*   Updated: 2024/02/19 16:12:14 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUESTPARSER_HPP
# define HTTPREQUESTPARSER_HPP

#include <string>
#include <vector>
#include <sstream>

#include "Utils/Utils.hpp"
#include "HTTP/HTTPRequest.hpp"
#include "HTTP/BodyParser.hpp"
#include "HTTP/HeaderParser.hpp"
#include "HTTP/RequestLineParser.hpp"

namespace webserv
{

class HTTPRequestParser
{
private:
    enum status{
        _requestLine     = 1 ,
        _header          = 2 ,
        _requestBody     = 4 ,
        _parseComplete   = 5 ,
        _badRequest      = 6 ,
    };

public:
    HTTPRequestParser();

    Byte* getBuffer();
    void parse(uint32 len);
    inline HTTPRequest& request() { return m_request; };

    inline bool isRequestLineComplete() { return (m_status > _requestLine); };
    inline bool isHeaderComplete()      { return (m_status > _header);      };
    inline bool isBodyComplete()        { return (m_status > _requestBody); };

private:
    HTTPRequest m_request;

    RequestLineParser m_requestLineParser;
    HeaderParser m_headerParser;
    BodyParser m_bodyParser;
    
    int m_status;
    uint64 m_idx;
    std::string m_hex;
    std::vector<Byte> m_buffer;
};

}

#endif
