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
#include "HTTP/HTTP.hpp"

namespace webserv
{


class HTTPRequestParser
{
private:
    // enum class was c++11 extension
    enum status{
        _requestMethod =0,
        _slash,
        _uri,
        _HTTP,
        _verMajor,
        _dot =5,
        _verMinor,
        _endRequest,
        _headerKey,
        _headerValue,
        _badRequest =10,
        _parseComplete,
        _statusCount
    };
public:
    HTTPRequestParser();
    Byte* getBuffer();
    void parse(uint32 len);
    inline bool isComplete() {return (m_status == _parseComplete); };
    HTTPRequest getParsed();

private:
    void checkCRLF(Byte c, int successStatus);
    void initHeaderSet();
    void initParser();
    void decodeHex(Byte c);

    HTTPRequest m_request;
    int m_status;
    std::string m_protocol;
    bool m_foundCR;
    std::string m_key;
    std::string m_value;
    int m_idx;
    std::string m_hex;
    std::vector<Byte> m_buffer;
};
}

#endif // HTTPREQUESTPARSER_HPP
