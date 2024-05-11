/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:31:49 by tchoquet          #+#    #+#             */
/*   Updated: 2024/05/08 18:43:41 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/HTTPResponse.hpp"

namespace webserv
{

HTTPResponse::HTTPResponse(int code) : HTTPBase(), isComplete(false)
{
    setStatusCode(code);
}

void HTTPResponse::setStatusCode(uint32 code)
{
    statusCode = code;
    switch (code)
    {
    case 200:
        statusDescription = "OK";
        break;
    case 201:
        statusDescription = "Created";
        break;
    case 204:
        statusDescription = "No Content";
        break;
    case 301:
        statusDescription = "Moved Permanently";
        break;
    case 302:
        statusDescription = "Found";
        break;
    case 400:
        statusDescription = "Bad Request";
        break;
    case 403:
        statusDescription = "Forbidden";
        break;
    case 404:
        statusDescription = "Not Found";
        break;
    case 405:
        statusDescription = "Method Not Allowed";
        break;
    case 413:
        statusDescription = "Content Too Large";
        break;
    case 500:
        statusDescription = "Internal Server Error";
        break;
    case 501:
        statusDescription = "Not Implemented";
        break;
    case 502:
        statusDescription = "Bad Gateway";
        break;
    case 504:
        statusDescription = "Gateway Time-out";
        break;
    case 505:
        statusDescription = "HTTP Version Not Supported";
        break;
    default:
        statusDescription = "Unkown Error";
        break;
    }
}

void HTTPResponse::makeBuiltInResponse(uint32 code)
{
    setStatusCode(code);
    std::string bodyStr = BUILT_IN_ERROR_PAGE(statusCode, statusDescription);
    body.reserve(bodyStr.size());
    for (std::string::const_iterator c = bodyStr.begin(); c != bodyStr.end(); ++c)
        body.push_back(*c);
    headers["Content-Type"] = "text/html";
    headers["Content-Length"] = to_string(body.size());
    isComplete = true;
}

void HTTPResponse::getRaw(std::vector<Byte>& raw) const
{
    std::string firstLine = httpVersionStr() + ' ' + to_string(statusCode) + ' ' + statusDescription;
    
    uint64 headLen = 0;
    headLen += firstLine.size() + 2; // firstLine\r\n
    for (std::map<std::string, std::string>::const_iterator curr = headers.begin(); curr != headers.end(); ++curr)
        headLen += curr->first.size() + curr->second.size() + 4; // "first: second\r\n"
    headLen += 2; // \r\n

    raw.resize(headLen + body.size());

    uint64 i = 0;
    for (std::string::iterator c = firstLine.begin(); c != firstLine.end(); ++c)
        raw[i++] = *c;
    raw[i++] = '\r'; raw[i++] = '\n';
    for (std::map<std::string, std::string>::const_iterator curr = headers.begin(); curr != headers.end(); ++curr)
    {
        for (std::string::const_iterator c = curr->first.begin(); c != curr->first.end(); ++c)
            raw[i++] = *c;
        raw[i++] = ':'; raw[i++] = ' ';
        for (std::string::const_iterator c = curr->second.begin(); c != curr->second.end(); ++c)
            raw[i++] = *c;
        raw[i++] = '\r'; raw[i++] = '\n';
    }
    raw[i++] = '\r'; raw[i++] = '\n';

    for (std::vector<Byte>::const_iterator c = body.begin(); c != body.end(); ++c)
        raw[i++] = *c;
}

}