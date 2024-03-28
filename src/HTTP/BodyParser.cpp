/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BodyParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sotanaka <sotanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:27:27 by ekamada           #+#    #+#             */
/*   Updated: 2024/03/28 14:56:19 by sotanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BodyParser.hpp"

namespace webserv
{

BodyParser::BodyParser(std::vector<Byte>& bodyDst)
    : m_body(&bodyDst), m_status(_requestBody), m_contentLength(0), m_isChunkLen(true)
{
}

void BodyParser::setContentLength(uint64 len)
{
    m_body->reserve(len);
    m_contentLength = len;
}

void BodyParser::parse(Byte c)
{
	switch(m_status)
	{
		case _requestBody:
			m_body->push_back(c);

			if (m_contentLength > 0 && m_body->size() >= m_contentLength)
				m_status = _parseComplete;

		default:
			return;
	}
}

void BodyParser::checkCRLF(Byte c, int successStatus)
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

uint64 BodyParser::hexStringToUint64(const std::string& str)
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

void BodyParser::parseChunk(Byte c)
{
	switch(m_status)
	{
		case _requestBody:
			if (m_isChunkLen)
			{
				if (c == '\r' || c == '\n')
				{
					checkCRLF(c, false);
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
					m_body->push_back(c);
					--m_contentLength;
				}
				else
				{
					checkCRLF(c, true);
					if (m_isChunkLen)
						m_chunkLenStr.clear();
				}
			}
			break;
		case _endUnchunk:
			checkCRLF(c, true);
			if (m_isChunkLen)
				m_status  = _parseComplete;
			break;
		default:
			return;
	}
}
// Thomas, if _badrequest, do we need to clear the body in the Parser function?
}
