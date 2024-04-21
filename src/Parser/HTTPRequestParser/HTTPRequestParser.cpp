/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/19 18:35:15 by ekamada           #+#    #+#             */
/*   Updated: 2024/04/17 15:10:17 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequestParser.hpp"

namespace webserv
{

HTTPRequestParser::HTTPRequestParser(const HTTPRequestPtr& request)
    : m_request(request),
      m_status(_requestLine),
      m_requestLineParser(m_request->method, m_request->uri, m_request->params, m_request->query, m_request->verMajor, m_request->verMinor),
      m_headerParser(m_request->headers)
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
    m_curr = m_buffer.begin();
    continueParsing();
}

void HTTPRequestParser::continueParsing()
{
	for (;;)
	{
        switch (m_status)
        {
        case _requestLine:
            if (m_requestLineParser.isComplete())
            {
                if (m_requestLineParser.isBadRequest())
                    m_status = _badRequest;
                else
                    m_status = _requestHeaders;
                goto ret;
            }
            else if (m_curr == m_buffer.end())
                goto ret;
            else
                m_requestLineParser.parse(*m_curr++);
            break;

        case _requestHeaders:
            if (m_headerParser.isComplete())
            {
                if (m_headerParser.isBadRequest())
                    m_status = _badRequest;
                else
                    m_status = _requestBody;
                goto ret;
            }
            else if (m_curr == m_buffer.end())
                goto ret;
            else
                m_headerParser.parse(*m_curr++);
            break;

        case _requestBody:
            if (!m_bodyParser)
            {
                if (m_request->isChunk)
                    m_bodyParser = new ChunkedBodyParser(m_request->body);
                else
                    m_bodyParser = new BodyParser(m_request->body, m_request->contentLength);
            }

            if (m_bodyParser->isComplete())
            {
                if (m_bodyParser->isBadRequest())
                    m_status = _badRequest;
                else
                    m_status = _parseComplete;
                goto ret;
            }
            else if (m_curr == m_buffer.end())
                goto ret;
            else
                m_bodyParser->parse(*m_curr++);
            break;

        default:
            return;
        }
    }

ret:
    if (m_curr == m_buffer.end())
        m_buffer.clear();
    else
        m_buffer = std::vector<Byte>(m_curr, m_buffer.end());
    m_curr = m_buffer.begin();
}

void HTTPRequestParser::nextRequest(const HTTPRequestPtr& request)
{
    m_request = request;

    m_status = _requestLine;

    m_requestLineParser = RequestLineParser(m_request->method, m_request->uri, m_request->params, m_request->query, m_request->verMajor, m_request->verMinor);
    m_headerParser = HTTPHeaderParser(m_request->headers);
    m_bodyParser = NULL;
    m_curr = m_buffer.begin();

    continueParsing();
}

}
