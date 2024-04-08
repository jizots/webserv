/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hotph <hotph@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/19 18:35:15 by ekamada           #+#    #+#             */
/*   Updated: 2024/04/07 17:11:07 by hotph            ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "HTTPRequestParser.hpp"
#include "MultipartFormParser.hpp"

namespace webserv
{

HTTPRequestParser::HTTPRequestParser(const HTTPRequestPtr& request)
    : m_request(request), m_headerParser(m_request->headers),
      m_status(_requestMethod), m_foundCR(false), m_isChunkLen(true), m_contentLength(0)
{
}

Byte* HTTPRequestParser::getBuffer()
{
    m_buffer.resize(BUFFER_SIZE);
    return m_buffer.data();
}

void HTTPRequestParser::parse(uint32 len)
{
    m_buffer.resize(len);
    continueParsing();
}

void HTTPRequestParser::parseMultipart(void)
{
	MultipartFormParser mfp;

	m_request->m_multipartFormDatas = mfp.parse(m_request->body, m_request->boundary);
	if (mfp.isBadRequest())
		m_status = _badRequest;
}

void HTTPRequestParser::continueParsing()
{
	for (std::vector<Byte>::iterator it = m_buffer.begin(); it != m_buffer.end(); ++it)
	{
		if (isRequestLineComplete() == false)
		{
			requestLineParse(*it);
			if (isRequestLineComplete())
			{
				m_buffer = std::vector<Byte>(++it, m_buffer.end());
				return;
			}
			continue;
		}

		if (isHeaderComplete() == false)
		{
			m_headerParser.parse(*it);

			if (m_headerParser.isComplete() == false)
				continue;

			if (m_headerParser.isBadRequest())
				m_status = _badRequest;
			else
				m_status = _requestBody;

			m_buffer = std::vector<Byte>(++it, m_buffer.end());
			return;
		}

		if (isBodyComplete() == false)
		{
			if (m_request->isChunk)
			{
				parseChunk(*it);
				if (isBodyComplete())
				{
					if (m_request->isMultipart)
						parseMultipart();
					if (++it != m_buffer.end())
						m_buffer = std::vector<Byte>(++it, m_buffer.end());
					else
						m_buffer.clear();
					return;
				}
			}
			else
			{
				if (m_request->body.size() >= m_request->contentLength)
				{
					m_status = _parseComplete;
					if (m_request->isMultipart)
						parseMultipart();
					if (++it != m_buffer.end())
						m_buffer = std::vector<Byte>(++it, m_buffer.end());
					else
						m_buffer.clear();
					return;
				}
				else
					m_request->body.push_back(*it);
			}
		}
	}

	if (m_request->isChunk == false && m_status == _requestBody && m_request->body.size() >= m_request->contentLength)
    {
		m_status = _parseComplete;
        if (m_request->isMultipart)
            parseMultipart();
    }

	if (m_status == _parseComplete)
		m_buffer.clear();
}

void HTTPRequestParser::nextRequest(const HTTPRequestPtr& request)
{
    m_request = request;
    m_headerParser = HeaderParser(m_request->headers);

    m_status = _requestMethod;
    m_hex.clear();
    m_foundCR = false;
    m_protocol.clear();
    m_isChunkLen = true;
    m_contentLength = 0;

    continueParsing();
}

}
