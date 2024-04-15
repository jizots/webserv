/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestLineParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:27:27 by ekamada           #+#    #+#             */
/*   Updated: 2024/04/06 18:39:15 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser/HTTPRequestParser/HTTPRequestParser.hpp"

namespace webserv
{

HTTPRequestParser::RequestLineParser::RequestLineParser(std::string& methodDst, std::string& uriDst, std::string& paramsDst, std::string& queryDst, uint8& verMajorDst, uint8& verMinorDst)
    : m_method(&methodDst), m_uri(&uriDst), m_params(&paramsDst), m_query(&queryDst), m_verMajor(&verMajorDst), m_verMinor(&verMinorDst),
      m_status(_requestMethod), m_foundCR(false)
{
}

void HTTPRequestParser::RequestLineParser::parse(Byte c) 
{
    switch(m_status)
    {
        case _requestMethod:
            if (m_method->size() == 0 && ( c == '\r' || c == '\n'))
            {
                checkCRLF(c, _requestMethod);
                break;
            }

            *m_method += c;
            if (*m_method == "GET " || *m_method == "POST " || *m_method == "DELETE ")
            {
                *m_method = m_method->substr(0, m_method->size() - 1);
                m_status = _slash;
            }
            else if (m_method->size() > 6)
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
            c = tolower(c);
            if (c == '%' || !m_hex.empty())
                decodeHex(c, *m_uri);
            else if (c == '?')
                m_status = _query;
            else if (c == ';')
                m_status = _params;
            else if (c == ' ')
                m_status = _HTTP;
            else if (IS_PCHAR(c))
                *m_uri += c;
            else
                m_status = _badRequest;
            break;

        case _params:
            if (c == '%' || !m_hex.empty())
                decodeHex(c, *m_params);
            else if (c == '?')
                m_status = _query;
            else if (c == ' ')
                m_status = _HTTP;
            else if (IS_PCHAR(c) || c == '/')
                *m_params += c;
            else
                m_status = _badRequest;
            break;

        case _query:
            if (c == '%' || !m_hex.empty()) 
                decodeHex(c, *m_query);
            else if (c == ' ')
                m_status = _HTTP;
            else if (IS_UN_RESERVED(c) || IS_RESERVED(c))
                *m_query += c;
            else
                m_status = _badRequest;
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

void HTTPRequestParser::RequestLineParser::decodeHex(Byte c, std::string& dst)
{
    if (m_hex.empty() && c == '%')
        m_hex += "%";
    else if (m_hex == "%")
    {
        m_hex = "";
        if (c >= '2' && c <= '7')
            m_hex += c;
        else
            m_status = _badRequest;
    }
    else if (isdigit(c) || (c >= 'a' && c <= 'z'))
    {
        m_hex += c;
        std::stringstream ss;
        ss << std::hex << m_hex;
        int result = 0;
        ss >> result;
        if (IS_PRINTABLE_ASCII(static_cast<char>(result)))
        {
            dst += static_cast<char>(result);
            m_hex = "";
        }
        else
            m_status = _badRequest;
    }
    else
        m_status = _badRequest;
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
