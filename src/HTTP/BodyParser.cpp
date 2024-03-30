/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BodyParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:27:27 by ekamada           #+#    #+#             */
/*   Updated: 2024/03/28 18:33:25 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/HTTPRequestParser.hpp"

namespace webserv
{

void HTTPRequestParser::checkCRLFChunk(Byte c, int successStatus)
{
	if (c == '\r' && !m_foundCR && !m_chunkLenStr.empty())
		m_foundCR = true;
	else if (c == '\n' && m_foundCR && !m_chunkLenStr.empty())
	{
		m_isChunkLen = successStatus;
		m_foundCR = false;
	}
	else
		m_status = _badRequest;
}

uint64 HTTPRequestParser::hexStringToUint64(const std::string& str)
{
	uint64	result = 0;
	std::istringstream iss(str);

	for(std::string::size_type i = 0; i < str.size(); ++i)
	{
		if (!std::isxdigit(str[i]))
			m_status = _badRequest;
	}
	iss >> std::hex >> result;
	if (iss.fail() || !iss.eof())
		m_status = _badRequest;
	return (result);
};

void HTTPRequestParser::parseChunk(Byte c)
{
	switch(m_status)
	{
		case _requestBody:
			if (m_isChunkLen)
			{
				if (c == '\r' || c == '\n')
				{
					checkCRLFChunk(c, false);
					if (!m_isChunkLen)
					{
						m_contentLength = hexStringToUint64(m_chunkLenStr);
						if (m_status != _badRequest && m_contentLength == 0)
							m_status = _endUnchunk;
					}
				}
				else
					m_chunkLenStr += c;
			}
			else
			{
				if (0 < m_contentLength)
				{
					m_request->body.push_back(c);
					--m_contentLength;
				}
				else
				{
					checkCRLFChunk(c, true);
					if (m_isChunkLen)
						m_chunkLenStr.clear();
				}
			}
			break;
		case _endUnchunk:
			checkCRLFChunk(c, true);
			if (m_isChunkLen)
				m_status  = _parseComplete;
			break;
		default:
			return;
	}
}
}
