/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPHeaderParser.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:27:20 by ekamada           #+#    #+#             */
/*   Updated: 2024/04/06 18:38:43 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser/HTTPHeaderParser/HTTPHeaderParser.hpp"

namespace webserv
{

HTTPHeaderParser::HTTPHeaderParser(std::map<std::string, std::string>& header)
    : m_header(&header), m_status(_headerKey), m_foundCR(false)
{
}

void HTTPHeaderParser::parse(Byte c)
{
	switch (m_status)
    {
		case _headerKey:
			c = tolower(c);

			if (m_key == "" && (c == '\r' || c == '\n'))
				checkCRLF(c, _parseComplete);

			else if (c == ':')
                m_status = _headerValue;

			else if (!IS_TOKEN(c))
				m_status = _badRequest;

			else m_key += c;

			break;

		case _headerValue:
			if (c == ' ' || IS_PRINTABLE_ASCII(c))
                m_value += c;

			else if (c == '\r' || c == '\n')
                checkCRLF(c, _headerKey);

			else
				m_status = _badRequest;
            
			break;
	};
}

void HTTPHeaderParser::checkCRLF(Byte c, int successStatus)
{
	if (c == '\r' && !m_foundCR)
        m_foundCR = true;

	else if (c == '\n' && m_foundCR)
    {
		if (m_key + m_value != "")
        {
			if (m_header->find(m_key) != m_header->end())
			{
				m_status = _badRequest;
				return ;
			}

			(*m_header)[m_key] = trimCharacters(m_value, " \t");
		}
        
		m_key = "";
        m_value = "";
        m_foundCR = false;

		m_status = successStatus;
	}
	else
		m_status = _badRequest;
}

}
