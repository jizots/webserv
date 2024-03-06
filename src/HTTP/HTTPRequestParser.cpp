/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/19 18:35:15 by ekamada           #+#    #+#             */
/*   Updated: 2024/03/04 08:36:30 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequestParser.hpp"
#include "ConfigParser/Utils.hpp"
namespace webserv
{

HTTPRequestParser::HTTPRequestParser() : m_idx(0) { initParser(); }

bool isUnReserved(Byte c) {
	std::string other = "!*`(),$-_.";
	return (std::isalpha(c) || std::isdigit(c) || other.find(c) != std::string::npos);
}
bool isPchar(Byte c) {
	std::string pchar = ":@&=+/";
	return (isUnReserved(c) || pchar.find(c) != std::string::npos );
}

bool isToken(Byte c) {
	std::string token = "!#$%&'*+-.^_`|~";
	return (std::isalpha(c) || std::isdigit(c) || token.find(c) != std::string::npos);
}

bool isPrintableAscii(Byte c) { return (c >= ' ' && c <= '~'); }


void HTTPRequestParser::initHeaderSet() {
		m_key = "";
		m_value = "";
		m_foundCR = false;
}

Byte* HTTPRequestParser::getBuffer() {
	m_buffer.resize(m_buffer.size() + BUFFER_SIZE);
	return &m_buffer[m_buffer.size() - BUFFER_SIZE];
}

void HTTPRequestParser::initParser(){
	initHeaderSet();
	m_status = _requestMethod;
	m_request.method = "";
	m_request.requestedFile = "/";
	m_request.isBadRequest = false;
	m_protocol = "";
	m_buffer = std::vector<Byte>(m_buffer.begin() + m_idx, m_buffer.end());
	m_idx = 0;
	m_hex = "";
	m_contentLength = 0;
};

void HTTPRequestParser::decodeHex(Byte c) {
	if (m_hex.empty() && c == '%') m_hex += "%";
	else if (m_hex == "%") {
		m_hex = "";
		if (c >= '2' && c <= '7') m_hex += c;
		else m_status = _badRequest;
	}
	else if (isdigit(c) || (c >= 'a' && c <= 'z'))
	{
		m_hex += c;
		std::stringstream ss;
		ss << std::hex << m_hex;
		int result = 0;
		ss >> result;
		if (isPrintableAscii(static_cast<char>(result)))
		{
			m_request.requestedFile += static_cast<char>(result);
			m_hex = "";
		}
		else m_status = _badRequest;
	}
	else m_status = _badRequest;

}

std::string trimOptionalSpace(const std::string& str) {
	const std::string space = " \t";
	const size_t start = str.find_first_not_of(space);
	if (start == std::string::npos) return "";
	const size_t end = str.find_last_not_of(space);
	return str.substr(start, end - start + 1);
}
void HTTPRequestParser::checkCRLF(Byte c, int successStatus) {
	if (c == '\r' && !m_foundCR) m_foundCR = true;
	else if (c == '\n' && m_foundCR) {
		m_status = successStatus;
		if (m_key + m_value != "")
			m_request[m_key] = trimOptionalSpace(m_value);
		initHeaderSet();
	}
	else {
		m_status = _badRequest;
		initHeaderSet();
	}
}

void HTTPRequestParser::parse(uint32 len) {
	m_buffer.resize(m_buffer.size() + len - BUFFER_SIZE);
	while (m_idx < m_buffer.size() && m_status != _parseComplete && m_status != _badRequest) {
		int idx = m_idx++;
		switch (m_status) {
			case _requestMethod:
				m_request.method += m_buffer[idx];
				if (m_request.method == "GET " || m_request.method == "POST " || m_request.method == "DELETE ")
				{
                    m_request.method = m_request.method.substr(0, m_request.method.size() - 1);
					m_status = _slash;
				}
				else if (m_request.method.size() > 6)
                    m_status = _badRequest;
				break;
			case _slash:
				if (m_buffer[idx] == '/') m_status = _uri;
				else m_status = _badRequest;
				break;
			case _uri:
				m_buffer[idx] = tolower(m_buffer[idx]);
				if (m_buffer[idx] == '%' || !m_hex.empty()) {
					decodeHex(m_buffer[idx]);
				}
				else if (m_buffer[idx] == ' ') m_status = _HTTP;
				else if (isPchar(m_buffer[idx])) m_request.requestedFile += m_buffer[idx];
				else m_status = _badRequest;
				break;
			case _HTTP:
				m_protocol += m_buffer[idx];
				if (m_protocol == "HTTP/") m_status = _verMajor;
				else if (m_protocol[m_protocol.size() - 1] != "HTTP/"[m_protocol.size() - 1])
                    m_status = _badRequest;
				break;
			case _verMajor:
				if (std::isdigit(m_buffer[idx])) {
					m_request.httpVersionMajor = m_buffer[idx] - '0';
					m_status = _dot;
				}
				else m_status = _badRequest;
				break;
			case _dot:
				if (m_buffer[idx] == '.') m_status = _verMinor;
				else m_status = _badRequest;
				break;
			case _verMinor:
				if (std::isdigit(m_buffer[idx])) {
					m_request.httpVersionMinor = m_buffer[idx] - '0';
					m_status = _endRequest;
				}
				else m_status = _badRequest;
				break;
			case _endRequest:
				checkCRLF(m_buffer[idx], _headerKey);
				break;
			case _headerKey:
				m_buffer[idx] = tolower(m_buffer[idx]);
				if (m_key == "" && (m_buffer[idx] == '\r' || m_buffer[idx] == '\n')) {
					if (m_request.method == "GET" || m_request["content-length"] == "")
						checkCRLF(m_buffer[idx], _parseComplete);
					else checkCRLF(m_buffer[idx], _requestBody);
				}
				else if (m_buffer[idx] == ':') m_status = _headerValue;
				else if (!isToken(m_buffer[idx])) m_status = _badRequest;
				else m_key += m_buffer[idx];
				break;
			case _headerValue:
				m_buffer[idx] = tolower(m_buffer[idx]);
				if (m_buffer[idx] == ' ' || isPrintableAscii(m_buffer[idx])) m_value += m_buffer[idx];
				else if (m_buffer[idx] == '\r' || m_buffer[idx] == '\n') checkCRLF(m_buffer[idx], _headerKey);
				else {
					m_status = _badRequest;
					initHeaderSet();
				}
				break;
			case _requestBody:
				if (!m_contentLength)
					m_contentLength = convertStrToType<int>(m_request["content-length"], isInt);
				m_request.body.push_back(m_buffer[idx]);
				if (m_request.body.size() == m_contentLength)
                    m_status = _parseComplete;

		}
	}
	if (m_status == _parseComplete && m_request["host"] == "")
		m_request.isBadRequest = true;
	if (m_status == _badRequest) {
		log << "BadRequest\n";
		m_request.isBadRequest = true;
		m_status = _parseComplete;
	}
}

HTTPRequest HTTPRequestParser::getParsed()
{
	HTTPRequest result = m_request;
	m_request.headers.clear();
	m_request.body.clear();
	initParser();
    return result;
}

}
