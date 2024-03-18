/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIReadTask.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:14:10 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/17 19:19:56 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/CGIReadTask.hpp"

#include <unistd.h>

#include "IO/IOManager.hpp"
#include "IO/ClientSocketWriteTask.hpp"

namespace webserv
{

CGIReadTask::CGIReadTask(int readFd, IWriteTask* cgiWritetask, const ClientSocketPtr& clientSocket, const HTTPResponsePtr& response)
    : m_readFd(readFd), m_cgiWritetask(cgiWritetask), m_clientSocket(clientSocket), m_response(response)
{
}

int CGIReadTask::fd()
{
    return m_readFd;
}

void CGIReadTask::read()
{
    ssize_t readLen = ::read(m_readFd, m_parser.getBuffer(), BUFFER_SIZE);

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
        goto badResponse;

    for (std::map<std::string, std::string>::const_iterator it = m_parser.header().begin(); it != m_parser.header().end(); ++it)
    {
        if (it->first == "status")
        {
            if (is<uint16>(it->second.substr(0, it->second.find_first_of(' '))) == false || to<uint16>(it->second.substr(0, it->second.find_first_of(' '))) > 599)
                goto badResponse;

            m_response->setStatusCode(to<uint16>(it->second.substr(0, it->second.find_first_of(' '))));
        }
        else
            m_response->headers[it->first] = it->second;
    }

    if (m_response->headers.find("content-length") == m_response->headers.end())
        m_response->headers["content-length"] = to_string(m_parser.body().size());

    m_response->body = m_parser.body();
    goto send;

badResponse:
    m_response->headers.clear();
    m_response->setStatusCode(502);
    m_response->makeBuiltInBody();
    m_response->headers["Content-Type"] = "text/html";
    m_response->headers["Content-Length"] = to_string(m_response->body.size());

send:
    m_response->isComplete = true;
    if (m_clientSocket->nextResponse() == m_response)
        IOManager::shared().insertWriteTask(new ClientSocketWriteTask(m_clientSocket, m_clientSocket->nextResponse()));

    IOManager::shared().eraseReadTask(this);
    IOManager::shared().eraseWriteTask(m_cgiWritetask);
}

CGIReadTask::~CGIReadTask()
{
    close(m_readFd);
}


}