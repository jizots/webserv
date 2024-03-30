/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIParser.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/19 18:35:15 by ekamada           #+#    #+#             */
/*   Updated: 2024/03/28 18:04:47 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/CGIParser.hpp"

namespace webserv
{

CGIParser::CGIParser(std::map<std::string, std::string>& headers, std::vector<Byte>& body)
    : m_headers(headers), m_body(body), m_headerParser(headers), m_status(_header)
{
}

Byte* CGIParser::getBuffer()
{
    m_buffer.resize(BUFFER_SIZE);
    return m_buffer.data();
}

void CGIParser::parse(uint32 len)
{
    if (len == 0)
    {
        if (m_status < _requestBody)
            m_status = _badRequest;
        else
            m_status = m_status == _badRequest ? _badRequest : _parseComplete;
    }
        
    m_buffer.resize(len);
    continueParsing();

}

void CGIParser::continueParsing()
{
    for (std::vector<Byte>::const_iterator it = m_buffer.begin(); it != m_buffer.end(); ++it)
    {
        switch (m_status)
        {
            case _header:
                m_headerParser.parse(*it);

                if (m_headerParser.isComplete() == false)
                    continue;

                if (m_headerParser.isBadRequest())
                    m_status = _badRequest;
                else
                {
                    std::map<std::string, std::string>::const_iterator contentLengthIt = m_headers.find("content-length");
                    if (contentLengthIt != m_headers.end() && is<uint64>(contentLengthIt->second))
                        m_contentLength = to<uint64>(contentLengthIt->second);
                    else
                        m_status = _requestBody;
                }

                break;

            case _requestBody:
                if (m_contentLength > 0 && m_body.size() >= m_contentLength)
				    m_status = _parseComplete;
                else
                    m_body.push_back(*it);
        }
    }
}

} // namespace webserv
