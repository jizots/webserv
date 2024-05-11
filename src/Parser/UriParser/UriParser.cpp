/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UriParser.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/17 16:11:38 by sotanaka          #+#    #+#             */
/*   Updated: 2024/05/09 19:27:52 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser/UriParser/UriPaser.hpp"

#include <algorithm>

namespace webserv
{

static std::pair<int, Byte> decodeHex(std::vector<Byte>::size_type& i, const std::vector<Byte>& buff)
{
    if (i >= buff.size() - 2 || IS_HEX(buff[i + 1]) == false || IS_HEX(buff[i + 2]) == false)
        return std::make_pair(-1, 0);
    i++;
    Byte decodedByte = 0;
    if (std::isdigit(buff[i]))
        decodedByte = buff[i] - '0';
    else
        decodedByte = 10 + std::tolower(buff[i]) - 'a';
    decodedByte *= 16;
    i++;
    if (std::isdigit(buff[i]))
        decodedByte += buff[i] - '0';
    else
        decodedByte += 10 + std::tolower(buff[i]) - 'a';
    return std::make_pair(0, static_cast<char>(decodedByte));
    i++;
}

static int appendParsed(const std::vector<Byte>& buff, std::string& dst)
{
    for (std::vector<Byte>::size_type i = 0; i < buff.size(); i++)
    {
        if (buff[i] == '%')
        {
            std::pair<int, Byte> err_decodedByte = decodeHex(i, buff);
            if (err_decodedByte.first != 0)
                return err_decodedByte.first;
            else
                dst += static_cast<char>(err_decodedByte.second);
        }
        else
            dst += static_cast<char>(buff[i]); // ? should we check for valid character ? 
    }
    return 0;
}

UriParser::UriParser(std::string& uriDst, std::string& paramsDst, std::string& queryDst)
    : m_uri(&uriDst), m_params(&paramsDst), m_query(&queryDst),
      m_status(_uri)
{
}

void UriParser::appendParsed(const std::vector<Byte>& buff)
{
    switch(m_status)
    {
        case _uri:
            // if (*buff.begin() == ';' || *buff.begin() == '?')
            //     m_status = _params;
            if (*buff.begin() == '?')
                m_status = _query;
            else if (m_uri->empty() && *buff.begin() != '/')
                m_status = _badURI;
            else if (webserv::appendParsed(buff, *m_uri) != 0)
                m_status = _badURI;
            else
                break;
            return appendParsed(buff);
                
        case _params:
            if (*buff.begin() == '?')
                m_status = _query;
            else if (webserv::appendParsed(m_params->empty() && *buff.begin() == ';' ? std::vector<Byte>(++buff.begin(), buff.end()) : buff, *m_params) != 0)
                m_status = _badURI;
            else
                break;
            return appendParsed(buff);

        case _query:
            if (webserv::appendParsed(m_query->empty() && *buff.begin() == '?' ? std::vector<Byte>(++buff.begin(), buff.end()) : buff, *m_query) != 0)
                m_status = _badURI;
            else
                break;
        default:
            break;
    }
}

void UriParser::parseString(const std::string& str)
{
    std::vector<Byte> uriBuff;

    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
    {
        if (*it == ' ')
        {
            m_status = _badURI;
            return;
        }
        if (IS_NGINX_UN_RESERVED(*it) == false)
        {
            if (uriBuff.empty() == false)
                appendParsed(uriBuff);
            if (isBadURI())
                return;
            else
                uriBuff = std::vector<Byte>(1, *it);
        }
        else
            uriBuff.push_back(*it);
    }
    if (uriBuff.empty() == false)
        appendParsed(uriBuff);
}

}