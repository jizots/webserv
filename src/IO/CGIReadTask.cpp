/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIReadTask.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:14:10 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/25 19:27:34 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/CGIReadTask.hpp"

#include <unistd.h>

#include "IO/IOManager.hpp"
#include "IO/ClientSocketWriteTask.hpp"
#include "RequestHandler/RequestHandler.hpp"

namespace webserv
{

CGIReadTask::CGIReadTask(CGIProgramPtr cgiProgram, IWriteTask* cgiWriteTask, const ClientSocketPtr& clientSocket, const HTTPResponsePtr& response, const RequestHandlerPtr& redirectionHandler)
    : m_cgiProgram(cgiProgram), m_cgiWriteTask(cgiWriteTask), m_clientSocket(clientSocket), m_response(response), m_redirectionHandler(redirectionHandler),
      m_parser(m_headers, m_body)
{
}

int CGIReadTask::fd()
{
    return m_cgiProgram->readFd();
}

void CGIReadTask::read()
{
    updateTimestamp();
    
    ssize_t readLen = ::read(fd(), m_parser.getBuffer(), BUFFER_SIZE);

    if (readLen < 0)
        throw std::runtime_error("read: " + std::string(std::strerror(errno)));
    
    if (readLen > 0)
        log << readLen << " Bytes read from cgi (fd: " << fd() << ")\n";
    else
        (void)(log << "EOF received from cgi (fd: " << fd() << ")\n");

    m_parser.parse(readLen);

    if (m_parser.isComplete() == false)
        return;

    if (m_parser.isBadRequest())
    {
        m_redirectionHandler->makeErrorResponse(502);
        m_redirectionHandler->runTasks(m_redirectionHandler);
        goto erase;
    }
    
    if (m_headers.size() == 1)
    {
        std::map<std::string, std::string>::const_iterator it = m_headers.find("location");
        if (it != m_headers.end())
        {
            m_redirectionHandler->internalRedirection("GET", it->second, ""); // TODO parse location
            m_redirectionHandler->runTasks(m_redirectionHandler);
            goto erase;
        }
            
    }

    for (std::map<std::string, std::string>::const_iterator it = m_headers.begin(); it != m_headers.end(); ++it)
    {
        if (it->first == "status")
        {
            if (is<uint16>(it->second.substr(0, it->second.find_first_of(' '))) == false || to<uint16>(it->second.substr(0, it->second.find_first_of(' '))) > 599)
            {
                m_redirectionHandler->makeErrorResponse(502);
                m_redirectionHandler->runTasks(m_redirectionHandler);
                goto erase;
            }

            m_response->setStatusCode(to<uint16>(it->second.substr(0, it->second.find_first_of(' '))));
        }
        else
            m_response->headers[it->first] = it->second;
    }

    if (m_response->headers.find("content-length") == m_response->headers.end())
        m_response->headers["content-length"] = to_string(m_body.size());

    std::swap(m_response->body, m_body);

    m_response->isComplete = true;
    if (m_clientSocket->nextResponse() == m_response)
        IOManager::shared().insertWriteTask(new ClientSocketWriteTask(m_clientSocket, m_clientSocket->nextResponse()));

erase:
    IOManager::shared().eraseReadTask(this);
    IOManager::shared().eraseWriteTask(m_cgiWriteTask);
}

CGIReadTask::~CGIReadTask()
{
    m_cgiProgram->closeReadFd();
}


}