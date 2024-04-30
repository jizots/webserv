/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIReadTask.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:14:10 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/28 14:52:31 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/IOTask/ReadTask/CGIReadTask.hpp"

#include <sys/wait.h>
#include <signal.h>
#include <algorithm>

#include "IO/IOManager.hpp"

namespace webserv
{

CGIReadTask::CGIReadTask(const FileDescriptor& fd, int pid, const HTTPResponsePtr& response, const RequestHandlerPtr& handler)
#ifndef NDEBUG
    : IReadTask(Duration::infinity()),
#else
    : IReadTask(Duration::seconds(5)),
#endif
      m_fd(fd), m_pid(pid), m_response(response), m_handler(handler), m_parser(m_headers), m_writeTaskPtr(NULL), m_status(header)
{
}

void CGIReadTask::read()
{
    updateTimestamp();
    
    ssize_t readLen = webserv::read(fd(), m_parser.getBuffer(), BUFFER_SIZE);

    if (readLen < 0)
    {
        log << "Error while reading cgi response (fd: " << fd() << "): " << std::strerror(errno) << '\n';
        m_handler->makeErrorResponse(502);
    }

    else if (readLen >= 0)
    {
        if (readLen == 0)
            log << "EOF received on fd: " << fd() << '\n';
        else
            log << readLen << " Bytes read on fd: " << fd() << '\n';

        m_parser.parse(static_cast<uint32>(readLen));

        switch (m_status)
        {
            case header:
                if (m_parser.isHeaderComplete() == false)
                    return;

                if (m_parser.isBadResponse())
                {
                    m_handler->makeErrorResponse(502);
                    break;
                }

                if (m_headers.size() == 1)
                {
                    std::map<std::string, std::string>::const_iterator it = m_headers.find("location");
                    if (it != m_headers.end())
                    {
                        std::string uri, params, query;
                        UriParser uriParser(uri, params, query);
                        uriParser.parseString(it->second);
                        if (uriParser.isBadRequest())
                            m_handler->makeErrorResponse(502);
                        else
                            m_handler->internalRedirection("GET", uri, params);
                        break;
                    }
                }

                if (int error = processHeaders())
                {
                    m_handler->makeErrorResponse(error);
                    break;
                }

                m_parser.continueParsing();

                m_status = body;
                /* fall through */
                
            case body:
                if (m_parser.isComplete() == false)
                    return;

                if (m_parser.isBadResponse())
                {
                    m_handler->makeErrorResponse(502);
                    break;
                }

                if (m_response->isChunk == false)
                    m_response->headers["content-length"] = to_string(m_response->body.size());
                m_response->isComplete = true;
        }
    }

    m_handler->runTasks(m_handler);
    IOManager::shared().eraseReadTask(this);
}

CGIReadTask::~CGIReadTask()
{
    if (::waitpid(m_pid, NULL, WNOHANG) == 0)
    {
        log << "Killing child process with pid: " << m_pid << '\n';
        kill(m_pid, SIGKILL);
        ::waitpid(m_pid, NULL, 0);
    }
};

int CGIReadTask::processHeaders()
{
    std::map<std::string, std::string>::const_iterator contentLengthIt = m_headers.find("content-length");
    if (contentLengthIt != m_headers.end())
    {
        if (is<uint64>(contentLengthIt->second) == false)
            return 502;

        uint64 contentLength = to<uint64>(contentLengthIt->second);
        if (contentLength > 0)
            m_parser.setBodyParser(new HTTPBodyParser(m_response->body, contentLength));
    }
    else
        m_parser.setBodyParser(new HTTPBodyParser(m_response->body));

    std::map<std::string, std::string>::const_iterator transEncodingIt = m_headers.find("transfer-encoding");
    if (transEncodingIt != m_headers.end())
    {
        std::vector<std::string> parsedTransferEncoding;
        parsedTransferEncoding = splitByChars(transEncodingIt->second, ",");

        for (std::vector<std::string>::iterator it = parsedTransferEncoding.begin(); it != parsedTransferEncoding.end(); ++it)
            *it = trimCharacters(*it, " \t");
        
        if (std::find(parsedTransferEncoding.begin(), parsedTransferEncoding.end(), "chunked") != parsedTransferEncoding.end())
            m_response->isChunk = true;
    }

    for (std::map<std::string, std::string>::const_iterator it = m_headers.begin(); it != m_headers.end(); ++it)
    {
        if (it->first == "status")
        {
            if (is<uint16>(it->second.substr(0, it->second.find_first_of(' '))) == false)
                return 502;
            m_response->setStatusCode(to<uint16>(it->second.substr(0, it->second.find_first_of(' '))));
        }
        else
            m_response->headers[it->first] = it->second;
    }

    return 0;
}

}