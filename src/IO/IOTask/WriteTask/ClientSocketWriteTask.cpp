/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocketWriteTask.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:15:37 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/28 14:48:48 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/IOTask/WriteTask/ClientSocketWriteTask.hpp"

#include <sys/socket.h>

#include "Utils/Macros.hpp"
#include "Utils/Logger.hpp"
#include "IO/IOManager.hpp"

namespace webserv
{

ClientSocketWriteTask::ClientSocketWriteTask(const ClientSocketPtr& clientSocket, const HTTPResponsePtr& resp)
#ifndef NDEBUG
    : IWriteTask(Duration::infinity()),
#else
    : IWriteTask(Duration::seconds(5)),
#endif
      m_clientSocket(clientSocket), m_idx(0)
{
    resp->getRaw(m_buffer);
}

void ClientSocketWriteTask::write()
{
    updateTimestamp();

    uint32 sendLen = send(fd(), m_buffer.data() + m_idx, m_buffer.size() - m_idx, 0);

    if (sendLen <= 0)
        log << "Error while sending data to client (fd: " << fd() << "): " << std::strerror(errno) << '\n';
    else
    {
        log << sendLen << " bytes send on client socket " << m_clientSocket->fileDescriptor() << '\n';
        m_idx += sendLen;
        if (m_idx < m_buffer.size())
            return;
    
        m_clientSocket->popResponse();
        HTTPResponsePtr nextResponse = m_clientSocket->nextResponse();
        if (nextResponse && nextResponse->isComplete)
            IOManager::shared().insertWriteTask(new ClientSocketWriteTask(m_clientSocket, m_clientSocket->nextResponse()));
    }

    IOManager::shared().eraseWriteTask(this);
}

}