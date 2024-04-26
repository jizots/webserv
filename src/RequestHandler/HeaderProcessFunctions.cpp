/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderProcessFunctions.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/21 15:22:51 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/23 12:55:25 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler/RequestHandler.hpp"


namespace webserv
{

static int parseCommaSeparatedHeader(const std::string& headerValue, std::vector<std::string>&);

int RequestHandler::processHostHeader()
{
    std::map<std::string, std::string>::const_iterator it = m_request->headers.find("host");
    if (it == m_request->headers.end())
        return 400;

    std::vector<std::string> strs = splitByChars(it->second, ":");
    m_request->port = 80;
    if (strs.size() == 0 || 2 < strs.size())
    {
        log << "parseHost(): Invalid Host field: " << it->second << "\n";
        return 400;
    }

    m_request->hostname = stringToLower(strs[0]);
    if (strs.size() == 2 && is<uint16>(strs[1]))
        m_request->port = to<uint16>(strs[1]);

    return 0;
}

int RequestHandler::processConnectionHeader()
{
    std::map<std::string, std::string>::const_iterator connectionIt = m_request->headers.find("connection");
    if (connectionIt != m_request->headers.end())
    {
        if (connectionIt->second == "close")
            m_response->headers["Connection"] = "close";
        else
        {
            std::vector<std::string> parsedValues;
            if(parseCommaSeparatedHeader(connectionIt->second, parsedValues) != 0)
                return 400;
            
            for (std::vector<std::string>::iterator parsedValuesIt = parsedValues.begin(); parsedValuesIt != parsedValues.end() ; ++parsedValuesIt)
            {
                if (*parsedValuesIt == "keep-alive")
                {
                    if(int error = processKeepAliveHeader())
                        return error;
                }
                else
                    return 501; // ? maybe other response code
            }   
        }
    }
    return 0;
}

int RequestHandler::processContentLengthHeader()
{
    std::map<std::string, std::string>::const_iterator contentLengthIt = m_request->headers.find("content-length");
    if (contentLengthIt != m_request->headers.end())
    {
        if (!is<uint64>(contentLengthIt->second))
            return 400;
        m_request->contentLength = to<uint64>(contentLengthIt->second);
        return 0;
    }
    return 0;
}

int RequestHandler::processTransferEncodingHeader()
{
    std::map<std::string, std::string>::const_iterator transferEncodingIt = m_request->headers.find("transfer-encoding");
    if (transferEncodingIt != m_request->headers.end())
    {
        std::vector<std::string> parsedTransferEncoding;
        if (parseCommaSeparatedHeader(transferEncodingIt->second, parsedTransferEncoding) != 0)
            return 400;
        
        for (std::vector<std::string>::iterator parsedTransferEncodingIt = parsedTransferEncoding.begin(); parsedTransferEncodingIt != parsedTransferEncoding.end() ; ++parsedTransferEncodingIt)
        {
            if (*parsedTransferEncodingIt == "chunked")
                m_request->isChunk = true;
            else
                return 501; // ? maybe other response code
        }
    }
    return 0;
}

int RequestHandler::processKeepAliveHeader()
{
    std::map<std::string, std::string>::const_iterator keepAliveIt = m_request->headers.find("keep-alive");
    if (keepAliveIt != m_request->headers.end())
    {
        std::vector<std::string> parsedKeepAlive;
        if(parseCommaSeparatedHeader(keepAliveIt->second, parsedKeepAlive) != 0)
            return 400;

        for (std::vector<std::string>::iterator parsedKeepAliveIt = parsedKeepAlive.begin(); parsedKeepAliveIt != parsedKeepAlive.end() ; ++parsedKeepAliveIt)
        {
            std::vector<std::string> splittedParsedKeepAlive = splitByChars(*parsedKeepAliveIt, "=");
            if (splittedParsedKeepAlive.size() != 2)
                return 400;

            if (splittedParsedKeepAlive[0] == "timeout")
            {
                if (is<uint32>(splittedParsedKeepAlive[1]) == false)
                    return 400;
                m_request->timeout = to<uint32>(splittedParsedKeepAlive[1]);
            }
            else
                return 501; // ? maybe other response code
        } 
    }
    return 0;
}

static int parseCommaSeparatedHeader(const std::string& headerValue, std::vector<std::string>& dst)
{
    dst = splitByChars(headerValue, ",");

    for (std::vector<std::string>::iterator it = dst.begin(); it != dst.end(); ++it)
        *it = trimCharacters(*it, " \t");

    return 0;
}

}