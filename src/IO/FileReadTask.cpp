/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileReadTask.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:08:53 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/25 19:30:13 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/FileReadTask.hpp"

#include <unistd.h>

#include "IO/IOManager.hpp"
#include "IO/ClientSocketWriteTask.hpp"

namespace webserv
{

FileReadTask::FileReadTask(DiskResourcePtr resource, const ClientSocketPtr& clientSocket, const HTTPResponsePtr& response)
    : m_resource(resource), m_clientSocket(clientSocket), m_response(response)
{
}

int FileReadTask::fd()
{
    return m_resource->readFd();
}

void FileReadTask::read()
{
    updateTimestamp();
    
    ssize_t readLen = ::read(fd(), m_response->body.data(), m_response->body.size());

    if (readLen < 0 || static_cast<uint64>(readLen) != m_response->body.size())
        throw std::runtime_error("read: " + std::string(std::strerror(errno)));

    log << readLen << " bytes read from file \"" << m_resource->path() << "\" (fd: " << fd() << ")\n";

    m_response->isComplete = true;
    if (m_clientSocket->nextResponse() == m_response)
        IOManager::shared().insertWriteTask(new ClientSocketWriteTask(m_clientSocket, m_clientSocket->nextResponse()));

    IOManager::shared().eraseReadTask(this);
}

}