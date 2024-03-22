/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: emukamada <emukamada@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/19 18:35:15 by ekamada           #+#    #+#             */
/*   Updated: 2024/03/22 14:05:34 by emukamada        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequestParser.hpp"

namespace webserv
{

HTTPRequestParser::HTTPRequestParser()
    : m_request(),
      m_requestLineParser(m_request),
      m_headerParser(m_request.headers),
      m_bodyParser(m_request.body),
      m_status(_requestLine),
      m_idx(0)
{
}

Byte* HTTPRequestParser::getBuffer()
{
    m_buffer.resize(m_buffer.size() + BUFFER_SIZE);
    return &m_buffer[m_buffer.size() - BUFFER_SIZE];
}

void HTTPRequestParser::parse(uint32 len)
{

    m_buffer.resize(m_buffer.size() + len - BUFFER_SIZE);

    while (m_idx < m_buffer.size() && m_status != _parseComplete && m_status != _badRequest)
    {
        int idx = m_idx++;

        switch (m_status)
        {
            case _requestLine:
                m_requestLineParser.parse(m_buffer[idx]);

                if (m_requestLineParser.isComplete())
                    m_status = _header;

                else if (m_request.isBadRequest)
                    m_status = _badRequest;

                break;

            case _header:
                m_headerParser.parse(m_buffer[idx]);

                if (m_headerParser.isComplete())
                {
                    m_status = _requestBody;

                    if (m_request.headers.find("host") == m_request.headers.end())
                        m_status = _badRequest;
                    else
                    {
                        std::map<std::string, std::string>::iterator it = m_request.headers.find("content-length");
                        if (it != m_request.headers.end())
                        {
                            if (!is<uint64>(m_request.headers["content-length"]))
                                m_status = _badRequest;
                            else
                            {
                                m_request.contentLength = to<uint64>(m_request.headers["content-length"]);
                                m_bodyParser.setContentLength(m_request.contentLength);
                            }
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
        }
    }

    if (m_status == _badRequest)
        m_request.isBadRequest = true;
}

}
