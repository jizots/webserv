/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocketReadTask.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:04:36 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/25 19:29:44 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/ClientSocketReadTask.hpp"

#include <sys/socket.h>

#include "IO/IOManager.hpp"

namespace webserv
{

ClientSocketReadTask::ClientSocketReadTask(const ClientSocketPtr& clientSocket)
    : IReadTask(Duration::seconds(5)), m_clientSocket(clientSocket),
      m_request(new HTTPRequest()), m_parser(m_request), m_handler(new RequestHandler(m_request, m_clientSocket)),
      m_status(requestLine)
{
}

int ClientSocketReadTask::fd()
{
    return m_clientSocket->fileDescriptor();
}

void ClientSocketReadTask::read()
{
    updateTimestamp();
    
    ssize_t recvLen = ::recv(fd(), m_parser.getBuffer(), BUFFER_SIZE, 0);

    if (recvLen > 0)
    {
        log << recvLen << " Bytes read on fd: " << fd() << '\n';

        m_parser.parse(static_cast<uint32>(recvLen));

        while (true)
        {
            switch (m_status)
            {
            case requestLine:
                if (m_parser.isRequestLineComplete() == false)
                    return;

                if (int error = m_handler->processRequestLine())
                {
                    m_handler->makeErrorResponse(error);
                    break;
                }

                m_status = header;
                /* fall through */

            case header:
                if (m_parser.isHeaderComplete() == false)
                    return;
                
                if (int error = m_handler->processHeaders())
                {
                    m_handler->makeErrorResponse(error);
                    break;
                }
                
                m_handler->makeResponse();

                if (m_handler->needBody() == false)
                    break;

                m_status = body;
                /* fall through */
                
            case body:
                if (m_parser.isBodyComplete() == false)
                    return;
            }

            m_handler->runTasks(m_handler);

            if (m_handler->shouldEndConnection() == false)
            {
                m_request = HTTPRequestPtr(new HTTPRequest());
                m_parser.nextRequest(m_request);
                m_handler = RequestHandlerPtr(new RequestHandler(m_request, m_clientSocket));
                m_status = requestLine;
            }
            else
            {
                IOManager::shared().eraseReadTask(this);
                return;
            }
        }
    }

    if (recvLen == 0)
        log << "EOF received on fd: " << fd() << '\n';

    IOManager::shared().eraseReadTask(this);
}

}