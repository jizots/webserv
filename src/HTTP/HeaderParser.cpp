/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: emukamada <emukamada@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:27:20 by ekamada           #+#    #+#             */
/*   Updated: 2024/03/20 15:05:24 by emukamada        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HeaderParser.hpp"

namespace webserv
{
HeaderParser::HeaderParser(std::map<std::string, std::string>& header)
: m_header(header), m_status(_headerKey), m_foundCR(false)
{
}

void HeaderParser::clearKeyValue() {
		m_key = "";
		m_value = "";
		m_foundCR = false;
}

void HeaderParser::checkCRLF(Byte c, int successStatus)
{
	if (c == '\r' && !m_foundCR) m_foundCR = true;
	else if (c == '\n' && m_foundCR) {
		m_status = successStatus;
		if (m_key + m_value != ""){
			if (m_header.find(m_key) != m_header.end())
			{
				m_status = _badRequest;
				return ;
			}
			m_header[m_key] = trimOptionalSpace(m_value);
		}
		clearKeyValue();
	}
	else
		m_status = _badRequest;
}

void HeaderParser::parse(Byte c)
{
	switch (m_status) {
		case _headerKey:
			c = tolower(c);
			if (m_key == "" && (c == '\r' || c == '\n'))
				checkCRLF(c, _parseComplete);
			else if (c == ':') m_status = _headerValue;
			else if (!isToken(c))
				m_status = _badRequest;
			else m_key += c;
			break;
		case _headerValue:
			c = tolower(c);
			if (c == ' ' || isPrintableAscii(c)) m_value += c;
			else if (c == '\r' || c == '\n') checkCRLF(c, _headerKey);
			else
				m_status = _badRequest;
			break;
	};
}


}
