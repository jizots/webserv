/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestLineParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:27:27 by ekamada           #+#    #+#             */
/*   Updated: 2024/05/09 17:36:01 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser/HTTPRequestParser/HTTPRequestParser.hpp"

namespace webserv
{

HTTPRequestParser::RequestLineParser::RequestLineParser(std::string& methodDst, std::string& uriDst, std::string& paramsDst, std::string& queryDst, uint8& verMajorDst, uint8& verMinorDst)
    : m_uriParser(UriParser(uriDst, paramsDst, queryDst)), m_method(&methodDst),
      m_verMajor(&verMajorDst), m_verMinor(&verMinorDst), m_status(_requestMethod), m_foundCR(false)
{
}

void HTTPRequestParser::RequestLineParser::parse(Byte c) 
{
    switch(m_status)
    {
        case _requestMethod:
            if (m_method->size() == 0 && (c == '\r' || c == '\n'))
            {
                checkCRLF(c, _requestMethod);
                break;
            }
            if (c >= 'A' && c <= 'Z')
                *m_method += c;
            else if (c == ' ' && m_method->size() > 0)
                m_status = _uri;
            else
                m_status = _badRequest;
            break;

        case _uri:
            if (m_uriBuff.empty() == false && IS_NGINX_UN_RESERVED(c) == false)
            {
                m_uriParser.appendParsed(m_uriBuff);
                if (m_uriParser.isBadURI())
                    m_status = _badRequest;
                else if (c == ' ')
                    m_status = _HTTP;
                else
                    m_uriBuff = std::vector<Byte>(1, c);
            }
            else if (c != ' ')
                m_uriBuff.push_back(c);
            break;

        case _HTTP:
            if (m_protocolRaw.empty() && c == ' ')
                break;
            m_protocolRaw += c;
            if (m_protocolRaw == "HTTP/")
                m_status = _verMajor;
            else if (m_protocolRaw[m_protocolRaw.size() - 1] != "HTTP/"[m_protocolRaw.size() - 1])
                m_status = _badRequest;
            break;

        case _verMajor:
            if (std::isdigit(c))
            {
                *m_verMajor = c - '0';
                m_status = _dot;
            }
            else
                m_status = _badRequest;
            break;

        case _dot:
            if (c == '.')
                m_status = _verMinor;
            else
                m_status = _badRequest;
            break;

        case _verMinor:
            if (std::isdigit(c))
            {
                *m_verMinor = c - '0';
                m_status = _requestLineEnd;
            }
            else
                m_status = _badRequest;
            break;
            
        case _requestLineEnd:
            checkCRLF(c, _parseComplete);
            break;

        default:
            break;
    }
}

void HTTPRequestParser::RequestLineParser::checkCRLF(Byte c, status successStatus)
{
    if (c == '\r' && !m_foundCR)
        m_foundCR = true;
    else
    {
        if (c == '\n' && m_foundCR)
            m_status = successStatus;
        else
            m_status = _badRequest;
        m_foundCR = false;
    }
}

}
