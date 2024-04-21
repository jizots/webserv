/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIReadTask.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:14:10 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/09 01:01:02 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/IOTask/ReadTask/CGIReadTask.hpp"

#include "IO/IOManager.hpp"

namespace webserv
{

CGIReadTask::CGIReadTask(const FileDescriptor& fd, const HTTPResponsePtr& response, const RequestHandlerPtr& handler)
    : m_fd(fd), m_response(response), m_handler(handler), m_parser(m_headers, m_response->body), m_writeTaskPtr(NULL), m_status(header)
{
}

void CGIReadTask::read()
{
    updateTimestamp();
    
    ssize_t readLen = webserv::read(fd(), m_parser.getBuffer(), BUFFER_SIZE);

    if (readLen < 0)
    {
        // log << "Error while reading cgi response (fd: " << fd() << "): " << std::strerror(errno) << '\n';
        // m_handler->makeErrorResponse(500);
        m_parser.clearBuffer();
        return;
    }

    if (readLen >= 0)
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
                        m_handler->internalRedirection("GET", it->second, ""); // TODO parse location
                        break;
                    }
                        
                }

                for (std::map<std::string, std::string>::const_iterator it = m_headers.begin(); it != m_headers.end(); ++it)
                {
                    if (it->first == "status")
                    {
                        if (is<uint16>(it->second.substr(0, it->second.find_first_of(' '))) == false || to<uint16>(it->second.substr(0, it->second.find_first_of(' '))) > 599)
                        {
                            m_handler->makeErrorResponse(502);
                            break;
                        }

                        m_response->setStatusCode(to<uint16>(it->second.substr(0, it->second.find_first_of(' '))));
                    }
                    else
                        m_response->headers[it->first] = it->second;
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

                m_response->headers["content-length"] = to_string(m_response->body.size());
                m_response->isComplete = true;
        }
    }

    m_handler->runTasks(m_handler);
    IOManager::shared().eraseReadTask(this);
}

}