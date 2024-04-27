/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPBodyParser.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/01 16:25:38 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/26 18:03:01 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser/HTTPBodyParser/HTTPBodyParser.hpp"

namespace webserv
{

HTTPBodyParser::HTTPBodyParser(std::vector<Byte>& bodyDst, uint64 contentLength)
    : m_body(&bodyDst), m_status(_body), m_contentLength(contentLength)
{
    if (contentLength == 0)
        m_status = _parseComplete;
    else
        m_body->reserve(m_contentLength);
}

HTTPBodyParser::HTTPBodyParser(std::vector<Byte>& bodyDst)
    : m_body(&bodyDst), m_status(_body), m_contentLength(0)
{
}

void HTTPBodyParser::parse(Byte c)
{
    m_body->push_back(c);
    if (m_contentLength > 0 && m_body->size() >= m_contentLength)
        m_status = _parseComplete;
}

void HTTPBodyParser::parseEOF()
{
    if (m_contentLength > 0 && m_body->size() != m_contentLength)
        m_status = _badRequest;
    else
        m_status = _parseComplete;
}

}