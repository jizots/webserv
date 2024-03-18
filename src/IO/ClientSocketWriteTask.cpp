/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocketWriteTask.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:15:37 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/06 17:39:07 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/ClientSocketWriteTask.hpp"

#include <sys/socket.h>

#include "IO/IOManager.hpp"

namespace webserv
{

ClientSocketWriteTask::ClientSocketWriteTask(const ClientSocketPtr& clientSocket, const HTTPResponsePtr& resp)
    : m_clientSocket(clientSocket), m_idx(0)
{
    resp->getRaw(m_buffer);
}

int ClientSocketWriteTask::fd()
{
    return m_clientSocket->fileDescriptor();
}

void ClientSocketWriteTask::write()
{
    uint32 sendLen = m_buffer.size() - m_idx > BUFFER_SIZE ? BUFFER_SIZE : m_buffer.size() - m_idx;

    if (::send(m_clientSocket->fileDescriptor(), &m_buffer[m_idx], sendLen, 0) != sendLen)
        throw std::runtime_error("send: " + std::string(std::strerror(errno)));
    
    m_idx += sendLen;
    if (m_idx == m_buffer.size())
    {
        log << m_buffer.size() << " bytes send on client socket " << m_clientSocket->fileDescriptor() << '\n';
        
        m_clientSocket->popResponse();
        HTTPResponsePtr nextResponse = m_clientSocket->nextResponse();
        if (nextResponse && nextResponse->isComplete)
            IOManager::shared().insertWriteTask(new ClientSocketWriteTask(m_clientSocket, m_clientSocket->nextResponse()));

        IOManager::shared().eraseWriteTask(this);
    }
}

}