/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTP.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/19 17:55:53 by tchoquet          #+#    #+#             */
/*   Updated: 2024/02/26 17:47:02 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/HTTP.hpp"

namespace webserv
{

HTTPResponse::HTTPResponse(int code) : HTTP(), isComplete(false), contentType(none)
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
    case 500:
        statusDescription = "Internal Server Error";
        break;
    }
}

void HTTPResponse::setContentType(const std::string& filePath)
{
    std::string ext = filePath.substr(filePath.find_last_of('.'));

    if (ext == ".html")
        contentType = HTTP::html;
    else if (ext == ".gif")
        contentType = HTTP::gif;
    else if (ext == ".ico")
        contentType = HTTP::icon;
    else
        contentType = HTTP::none;
}

void HTTPResponse::completeResponse()
{
    switch (contentType)
    {
        case HTTP::none:
            headers["Content-Length"] = "0";
            break;

        case HTTP::html:
            headers["Content-Type"] = "text/html";
            headers["Content-Length"] = std::to_string(body.size());
            break;

        case HTTP::gif:
            headers["Content-Type"] = "image/gif";
            headers["Content-Length"] = std::to_string(body.size());
            break;

        case HTTP::icon:
            headers["Content-Type"] = "image/vnd.microsoft.icon";
            headers["Content-Length"] = std::to_string(body.size());
            break;
    }
    isComplete = true;
}

void HTTPResponse::getRaw(std::vector<Byte>& raw) const
{
    std::string fstLine = firstLine();
    
    uint64 headLen = 0;
    headLen += fstLine.size() + 2; // fstLine\r\n
    for (std::map<std::string, std::string>::const_iterator curr = headers.begin(); curr != headers.end(); ++curr)
        headLen += curr->first.size() + curr->second.size() + 4; // "first: second\r\n"
    headLen += 2; // \r\n

    raw.resize(headLen + body.size());

    uint64 i = 0;
    for (std::string::iterator c = fstLine.begin(); c != fstLine.end(); ++c)
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