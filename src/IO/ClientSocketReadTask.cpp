/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocketReadTask.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:04:36 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/17 10:42:20 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/ClientSocketReadTask.hpp"

#include <sys/socket.h>

#include "IO/IOManager.hpp"

namespace webserv
{

ClientSocketReadTask::ClientSocketReadTask(const ClientSocketPtr& clientSocket)
    : m_clientSocket(clientSocket), m_parser(), m_handler(clientSocket), m_status(requestLine)
{
}

int ClientSocketReadTask::fd()
{
    return m_clientSocket->fileDescriptor();
}

void ClientSocketReadTask::read()
{
    ssize_t recvLen = ::recv(fd(), m_parser.getBuffer(), BUFFER_SIZE, 0);

    if (recvLen < 0)
        throw std::runtime_error("recv: " + std::string(std::strerror(errno)));

    if (recvLen > 0)
    {
        log << recvLen << " Bytes read on fd: " << fd() << '\n';
        m_parser.parse(static_cast<uint32>(recvLen));

        switch (m_status)
        {
        case requestLine:
            if (m_parser.isRequestLineComplete() == false)
                return;
            log << "processing requestLine\n";
            m_handler.processRequestLine(m_parser.request());
            if (m_parser.request().isBadRequest)
                break;
            m_status = header;
            /* fall through */

        case header:
            if (m_parser.isHeaderComplete() == false)
                return;
            log << "processing headers\n";
            m_handler.processHeaders(m_parser.request());
            if (m_handler.needBody() == false)
                break;
            m_status = body;
            /* fall through */
            
        case body:
            if (m_parser.isBodyComplete() == false)
                return;
        }

        log << "Handling request\n";
        m_handler.handleRequest(m_parser.request());

        if (m_handler.shouldEndConnection() == false)
            IOManager::shared().insertReadTask(new ClientSocketReadTask(m_clientSocket));
    }
    else
        log << "EOF received on fd: " << fd() << '\n';

    IOManager::shared().eraseReadTask(this);

    
    
}

}