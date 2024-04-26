/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UriParser.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/17 16:11:38 by sotanaka          #+#    #+#             */
/*   Updated: 2024/04/26 04:18:58 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser/UriParser/UriPaser.hpp"

#include <algorithm>

namespace webserv
{

UriParser::UriParser(std::string& uriDst, std::string& paramsDst, std::string& queryDst)
    : m_uri(&uriDst), m_params(&paramsDst), m_query(&queryDst),
      m_status(_uri)
{};

void UriParser::parse(Byte c)
{
    switch(m_status)
    {
        case _uri:
            c = std::tolower(c);
            if (c == '%' || !m_hex.empty())
                decodeHex(c, *m_uri);
            else if (c == '?')
                m_status = _query;
            else if (c == ';')
                m_status = _params;
            else if (c == ' ')
                m_status = _parseComplete;
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
                m_status = _parseComplete;
            else if (IS_PCHAR(c) || c == '/')
                *m_params += c;
            else
                m_status = _badRequest;
            break;

        case _query:
            if (c == '%' || !m_hex.empty()) 
                decodeHex(c, *m_query);
            else if (c == ' ')
                m_status = _parseComplete;
            else if (IS_UN_RESERVED(c) || IS_RESERVED(c))
                *m_query += c;
            else
                m_status = _badRequest;
            break;

        default:
            break;
    }

    if (m_status == _parseComplete)
    {
        std::vector<std::string> files = splitByChars(*m_uri, "/");
        if (std::find(files.begin(), files.end(), "..") != files.end())
            m_status = _badRequest;
    }
};

void UriParser::parseString(const std::string& uri)
{
    for (std::string::size_type i = 0; i < uri.size(); ++i)
    {
        parse(static_cast<Byte>(uri[i]));
        if (isBadRequest())
            break;
    }
}

void UriParser::decodeHex(Byte c, std::string& dst)
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

}