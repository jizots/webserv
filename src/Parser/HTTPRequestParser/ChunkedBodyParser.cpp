/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkedBodyParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:27:27 by ekamada           #+#    #+#             */
/*   Updated: 2024/04/08 17:44:21 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser/HTTPRequestParser/HTTPRequestParser.hpp"

namespace webserv
{

HTTPRequestParser::ChunkedBodyParser::ChunkedBodyParser(std::vector<Byte>& bodyDst)
    : BodyParser(bodyDst), m_isChunkLen(true), m_foundCR(false), m_endUnChunk(false)
{
}

void HTTPRequestParser::ChunkedBodyParser::parse(Byte c)
{
    if (m_endUnChunk)
    {
        checkCRLF(c, true);
        if (m_isChunkLen)
            m_status  = _parseComplete;
        return;;
    }

	switch(m_status)
	{
		case _body:
			if (m_isChunkLen)
			{
				if (c == '\r' || c == '\n')
				{
					checkCRLF(c, false);
					if (!m_isChunkLen)
					{
						m_contentLength = hexStringToUint64(m_chunkLenStr);
						if (m_status != _badRequest && m_contentLength == 0)
                            m_endUnChunk = true;
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
            
		default:
			return;
	}
}

void HTTPRequestParser::ChunkedBodyParser::checkCRLF(Byte c, bool successStatus)
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

uint64 HTTPRequestParser::ChunkedBodyParser::hexStringToUint64(const std::string& str)
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

}
