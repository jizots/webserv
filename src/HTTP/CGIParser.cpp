/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIParser.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/19 18:35:15 by ekamada           #+#    #+#             */
/*   Updated: 2024/03/23 16:46:18 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/CGIParser.hpp"

namespace webserv
{

CGIParser::CGIParser(std::map<std::string, std::string>& headers, std::vector<Byte>& body)
	: m_headers(headers),
      m_headerParser(headers), m_bodyParser(body),
      m_status(_header), m_idx(0)
{
}

Byte* CGIParser::getBuffer()
{
	m_buffer.resize(m_buffer.size() + BUFFER_SIZE);
	return &m_buffer[m_buffer.size() - BUFFER_SIZE];
}

void CGIParser::parse(uint32 len)
{
	m_buffer.resize(m_buffer.size() + len - BUFFER_SIZE);

    if (len == 0)
    {
        if (m_status < _requestBody)
            m_status = _badRequest;
        else
            m_status = m_status == _badRequest ? _badRequest : _parseComplete;
    }
    else
    {
        while (m_idx < m_buffer.size() && m_status != _parseComplete && m_status != _badRequest) {
            int idx = m_idx++;
            switch (m_status)
            {
                case _header:
                    m_headerParser.parse(m_buffer[idx]);

                    if (m_headerParser.isComplete())
                    {
                        m_status = _requestBody;
                        std::map<std::string, std::string>::const_iterator it = m_headers.find("content-length");
                        if (it != m_headers.end())
                        {
                            if (!is<uint64>(it->second))
                                m_status = _badRequest;
                            else
                                m_bodyParser.setContentLength(to<uint64>(it->second));
                        }
                    }

                    else if (m_headerParser.isBadRequest())
                        m_status = _badRequest;

                    break;

                case _requestBody:
                    m_bodyParser.parse(m_buffer[idx]);

                    if (m_bodyParser.isComplete())
                        m_status = _parseComplete;

                    else if (m_bodyParser.isBadRequest())
                        m_status = _badRequest;

                    break;
            }
        }
    }
}



}
