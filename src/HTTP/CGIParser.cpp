/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIParser.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: emukamada <emukamada@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/19 18:35:15 by ekamada           #+#    #+#             */
/*   Updated: 2024/03/22 14:06:00 by emukamada        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/CGIParser.hpp"

namespace webserv
{

CGIParser::CGIParser()
	: m_header(),
      m_body(),
      m_headerParser(m_header),
      m_bodyParser(m_body),
      m_contentLength(0),
      m_status(_header),
      m_idx(0)
{
}

Byte* CGIParser::getBuffer() {
	m_buffer.resize(m_buffer.size() + BUFFER_SIZE);
	return &m_buffer[m_buffer.size() - BUFFER_SIZE];
}

void CGIParser::parse(uint32 len) {

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
            switch (m_status) {
                case _header:
                    m_headerParser.parse(m_buffer[idx]);

                    if (m_headerParser.isComplete())
                    {
                        m_status = _requestBody;
                        std::map<std::string, std::string>::iterator it = m_header.find("content-length");
                        if (it != m_header.end())
                        {
                            if (!is<uint64>(m_header["content-length"]))
                                m_status = _badRequest;
                            else
                            {
                                m_contentLength = to<uint64>(m_header["content-length"]);
                                m_bodyParser.setContentLength(m_contentLength);
                            }
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
