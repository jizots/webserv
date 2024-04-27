/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIResponseParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/19 18:35:15 by ekamada           #+#    #+#             */
/*   Updated: 2024/04/26 18:12:32 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser/CGIResponseParser/CGIResponseParser.hpp"

#include "Utils/Macros.hpp"

namespace webserv
{

CGIResponseParser::CGIResponseParser(std::map<std::string, std::string>& headersDst)
    : m_status(_headers),
      m_headerParser(headersDst)
{
}

Byte* CGIResponseParser::getBuffer()
{
    m_buffer.resize(m_buffer.size() + BUFFER_SIZE);
    return &m_buffer[m_buffer.size() - BUFFER_SIZE];
}

void CGIResponseParser::parse(uint32 len)
{
    if (len == 0)
    {
        if (m_status == _body)
            m_bodyParser->parseEOF();
        else
            return (void)(m_status = _badResponse);
    }
    m_buffer.resize((m_buffer.size() - BUFFER_SIZE) + len);
    m_curr = m_buffer.begin();
    continueParsing();
}

void CGIResponseParser::continueParsing()
{
    for (;;)
    {
        switch (m_status)
        {
            case _headers:
                if (m_headerParser.isComplete())
                {
                    if (m_headerParser.isBadRequest())
                        m_status = _badResponse;
                    else
                        m_status = _body;
                    goto ret;
                }
                else if (m_curr == m_buffer.end())
                    goto ret;
                else
                    m_headerParser.parse(*m_curr++);
                break;

            case _body:
                if (m_bodyParser->isComplete())
                {
                    if (m_bodyParser->isBadRequest())
                        m_status = _badResponse;
                    else
                        m_status = _parseComplete;
                    goto ret;
                }
                else if (m_curr == m_buffer.end())
                    goto ret;
                else
                    m_bodyParser->parse(*m_curr++);
                break;
        }
    }

ret:
    if (m_curr == m_buffer.end())
        m_buffer.clear();
    else
        m_buffer = std::vector<Byte>(m_curr, m_buffer.end());
    m_curr = m_buffer.begin();
}

} // namespace webserv
