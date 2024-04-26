/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestLineParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:27:27 by ekamada           #+#    #+#             */
/*   Updated: 2024/04/26 11:23:37 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser/HTTPRequestParser/HTTPRequestParser.hpp"

namespace webserv
{

HTTPRequestParser::RequestLineParser::RequestLineParser(std::string& methodDst, std::string& uriDst, std::string& paramsDst, std::string& queryDst, uint8& verMajorDst, uint8& verMinorDst)
    : m_uriParser(UriParser(uriDst, paramsDst, queryDst)), m_method(&methodDst), m_uri(&uriDst),
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

            *m_method += std::toupper(c);
            if (c == ' ')
            {
                *m_method = m_method->substr(0, m_method->size() - 1);
                m_status = _slash;
            }
            else if (c == '\r' || c == '\n')
                m_status = _badRequest;
            break;

        case _slash:
            if (c == '/')
            {
                *m_uri += '/';
                m_status = _uri;
            }
            else
                m_status = _badRequest;
            break;

        case _uri:
            m_uriParser.parse(c);
            if (m_uriParser.isBadRequest())
                m_status = _badRequest;
            else if (m_uriParser.isComplete())
                m_status = _HTTP;
            break;

        case _HTTP:
            m_protocol += c;
            if (m_protocol == "HTTP/")
                m_status = _verMajor;
            else if (m_protocol[m_protocol.size() - 1] != "HTTP/"[m_protocol.size() - 1])
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
