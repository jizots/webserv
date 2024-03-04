/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IOTask.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/19 17:10:53 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/01 09:56:21 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IOTask.hpp"

#include <sys/socket.h>
#include <cstring>

#include "IOManager.hpp"
#include "ClientSocket.hpp"

namespace webserv
{

void ClientSocketReadTask::read(const IReadTaskPtr& _this)
{
    ssize_t recvLen = recv(fd(), m_parser.getBuffer(), BUFFER_SIZE, 0);

    if (recvLen < 0)
        throw std::runtime_error("recv: " + std::string(std::strerror(errno)));

    if (recvLen == 0)
        return (void)(log << "EOF received on fd: " << fd() << '\n');

    log << recvLen << " Bytes read on fd: " << fd();

    m_parser.parse(static_cast<uint32>(recvLen));
    if (m_parser.isComplete())
    {
        log << ". HTTP request complete\n";
        HTTPRequest req = m_parser.getParsed();
        m_clientSocket->masterSocket()->serverForName(req.isBadRequest ? "" : req.requestedServerHostName)->handleRequest(req, m_clientSocket);
        if (req.isBadRequest == false)
            IOManager::shared().insertReadTask(_this);
    }
    else
    {
        log << '\n';
        IOManager::shared().insertReadTask(_this);
    }
}

void FileReadTask::read(const IReadTaskPtr&)
{
    ssize_t readLen = ::read(m_fd, m_response->body.data(), m_response->body.size());

    if (readLen < 0 || static_cast<uint64>(readLen) != m_response->body.size())
        throw std::runtime_error("read: " + std::string(std::strerror(errno)));

    m_response->completeResponse();
    if (m_clientSocket->nextResponse() == m_response)
        IOManager::shared().insertWriteTask(new ClientSocketWriteTask(m_clientSocket, *m_clientSocket->nextResponse()));
}

void StdinReadTask::read(const IReadTaskPtr&)
{
    std::string cmd;
    
    std::getline(std::cin, cmd);
    if (cmd == "quit" || std::cin.eof())
        m_webserv.stop();
    else
        IOManager::shared().insertReadTask(new StdinReadTask(m_webserv));
}

ClientSocketWriteTask::ClientSocketWriteTask(const ClientSocketPtr& clientSocket, const HTTPResponse& resp)
    : m_clientSocket(clientSocket), m_idx(0)
{
    resp.getRaw(m_buffer);
}

void ClientSocketWriteTask::write(const IWriteTaskPtr& _this)
{
    uint32 sendLen = m_buffer.size() - m_idx > BUFFER_SIZE ? BUFFER_SIZE : m_buffer.size() - m_idx;

    if (send(m_clientSocket->fileDescriptor(), &m_buffer[m_idx], sendLen, 0) != sendLen)
        throw std::runtime_error("send: " + std::string(std::strerror(errno)));
    
    m_idx += sendLen;
    if (m_idx != m_buffer.size())
        IOManager::shared().insertWriteTask(_this);
    else
        log << m_buffer.size() << " bytes send on client socket " << m_clientSocket->fileDescriptor() << '\n';
}

ClientSocketWriteTask::~ClientSocketWriteTask()
{
    m_clientSocket->popResponse();
    HTTPResponsePtr nextResponse = m_clientSocket->nextResponse();
    if (nextResponse && nextResponse->isComplete)
        IOManager::shared().insertWriteTask(new ClientSocketWriteTask(m_clientSocket, *m_clientSocket->nextResponse()));
}

}