/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/19 18:35:15 by ekamada           #+#    #+#             */
/*   Updated: 2024/03/25 18:10:53 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequestParser.hpp"

namespace webserv
{

HTTPRequestParser::HTTPRequestParser(const HTTPRequestPtr& request)
	: m_request(request),
		m_headerParser(m_request->headers), m_bodyParser(m_request->body),
		m_idx(0), m_status(_requestMethod), m_foundCR(false), m_isChunk(false)
{
}

Byte* HTTPRequestParser::getBuffer()
{
    m_buffer.resize(m_buffer.size() + BUFFER_SIZE);
    return &m_buffer[m_buffer.size() - BUFFER_SIZE];
}

void HTTPRequestParser::parse(uint32 len)
{
	m_buffer.resize((m_buffer.size() - BUFFER_SIZE) + len);

	while (m_idx < m_buffer.size() && m_status < _parseComplete)
	{
		int idx = m_idx++;

		if (isRequestLineComplete() == false)
		{
			requestLineParse(m_buffer[idx]);
			continue;
		}

		switch (m_status)
		{
			case _header:
				m_headerParser.parse(m_buffer[idx]);

				if (m_headerParser.isComplete())
				{
					m_status = _parseComplete;

					std::map<std::string, std::string>::iterator it = m_request->headers.find("host");
					if (it == m_request->headers.end())
						m_status = _badRequest;
					else
					{
						m_request->host = it->second;

						it = m_request->headers.find("content-length");
						if (it != m_request->headers.end())
						{
							if (!is<uint64>(it->second))
								m_status = _badRequest;
							else
							{
								m_request->contentLength = to<uint64>(it->second);
								m_bodyParser.setContentLength(m_request->contentLength);
								m_status = _requestBody;
							}
						}
						it = m_request->headers.find("transfer-encoding");
						if (it != m_request->headers.end() && it->second == "chunked")
						{
							m_isChunk = true;
							m_status = _requestBody;
						}
					}
				}

				else if (m_headerParser.isBadRequest())
					m_status = _badRequest;

				break;

			case _requestBody:
				if (m_isChunk)
					m_bodyParser.parseChunk(m_buffer[idx]);
				else
					m_bodyParser.parse(m_buffer[idx]);

				if (m_bodyParser.isComplete())
					m_status = _parseComplete;

				else if (m_bodyParser.isBadRequest())
					m_status = _badRequest;
		}
	}

	if (m_status == _badRequest)
		m_request->isBadRequest = true;
}

void HTTPRequestParser::nextRequest(const HTTPRequestPtr& request)
{
    m_request = request;
    m_headerParser = HeaderParser(m_request->headers);
    m_bodyParser = BodyParser(m_request->body);

    std::vector<Byte> newBuffer(m_buffer.begin() + m_idx, m_buffer.end());
    std::swap(m_buffer, newBuffer);
    m_idx = 0;

    m_status = _requestMethod;
    m_hex.clear();
    m_foundCR = false;
    m_protocol.clear();
	m_isChunk = false;

    parse(BUFFER_SIZE);
}

}
