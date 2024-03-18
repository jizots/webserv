/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BodyParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:27:27 by ekamada           #+#    #+#             */
/*   Updated: 2024/03/17 18:19:55 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BodyParser.hpp"

namespace webserv
{

BodyParser::BodyParser(std::vector<Byte>& body): m_body(body), m_status(_requestBody), m_contentLength(0)
{
}

void BodyParser::setContentLength(uint64 len)
{
    m_body.reserve(len);
    m_contentLength = len;
}

void BodyParser::parse(Byte c)
{
	switch(m_status)
    {
		case _requestBody:
			m_body.push_back(c);

			if (m_contentLength > 0 && m_body.size() >= m_contentLength)
				m_status = _parseComplete;

        default:
            return;
	}
}


}
