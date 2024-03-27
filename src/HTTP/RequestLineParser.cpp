/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestLineParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:27:27 by ekamada           #+#    #+#             */
/*   Updated: 2024/03/25 16:44:47 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/HTTPRequestParser.hpp"

namespace webserv
{


void HTTPRequestParser::decodeHex(Byte c, std::string& dst)
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
        if (isPrintableAscii(static_cast<char>(result)))
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


void HTTPRequestParser::checkCRLF(Byte c, int successStatus)
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

void HTTPRequestParser::requestLineParse(Byte c) 
{
    switch(m_status)
    {
        case _requestMethod:
            if (m_request->method.size() == 0 && ( c == '\r' || c == '\n'))
            {
                checkCRLF(c, _requestMethod);
                break;
            }

            m_request->method += c;
            if (m_request->method == "GET " || m_request->method == "POST " || m_request->method == "DELETE ")
            {
                m_request->method = m_request->method.substr(0, m_request->method.size() - 1);
                m_status = _slash;
            }
            else if (m_request->method.size() > 6)
                m_status = _badRequest;
            break;

        case _slash:
            if (c == '/')
            {
                m_request->uri += '/';
                m_status = _uri;
            }
            else
                m_status = _badRequest;
            break;

        case _uri:
            c = tolower(c);
            if (c == '%' || !m_hex.empty())
                decodeHex(c, m_request->uri);
            else if (c == '?')
                m_status = _query;
            else if (c == ';')
                m_status = _params;
            else if (c == ' ')
                m_status = _HTTP;
            else if (isPchar(c))
                m_request->uri += c;
            else
                m_status = _badRequest;
            break;

        case _params:
            if (c == '%' || !m_hex.empty())
                decodeHex(c, m_request->params);
            else if (c == '?')
                m_status = _query;
            else if (c == ' ')
                m_status = _HTTP;
            else if (isPchar(c) || c == '/')
                m_request->params += c;
            else
                m_status = _badRequest;
            break;

        case _query:
            if (c == '%' || !m_hex.empty()) 
                decodeHex(c, m_request->query);
            else if (c == ' ')
                m_status = _HTTP;
            else if (isUnReserved(c) || isReserved(c))
                m_request->query += c;
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
                m_request->httpVersionMajor = c - '0';
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
                m_request->httpVersionMinor = c - '0';
                m_status = _requestLineEnd;
            }
            else
                m_status = _badRequest;
            break;
            
        case _requestLineEnd:
            checkCRLF(c, _header);
            break;
    }
}

}
