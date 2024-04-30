/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocketReadTask.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:04:36 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/28 14:33:21 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/IOTask/ReadTask/ClientSocketReadTask.hpp"

#include <cstring>
#include <cerrno>

#include "IO/IOManager.hpp"

namespace webserv
{

ClientSocketReadTask::ClientSocketReadTask(const ClientSocketPtr& clientSocket)
#ifndef NDEBUG
    : IReadTask(Duration::infinity()),
#else
    : IReadTask(Duration::seconds(5)),
#endif
      m_clientSocket(clientSocket),
      m_request(new HTTPRequest()), m_parser(m_request), m_handler(new RequestHandler(m_request, m_clientSocket)),
      m_status(requestLine)
{
}

void ClientSocketReadTask::read()
{
    updateTimestamp();
    
    ssize_t recvLen = recv(fd(), m_parser.getBuffer(), BUFFER_SIZE, 0);

    if (recvLen < 0)
        log << "Error while reading client request (fd: " << fd() << "): " << std::strerror(errno) << '\n';

    else if (recvLen == 0)
        log << "EOF received on fd: " << fd() << '\n';

    else if (recvLen > 0)
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
                    
                if (m_parser.isBadRequest())
                {
                    m_handler->makeErrorResponse(400);
                    break;
                }
                if (int error = m_handler->processRequestLine())
                {
                    m_handler->makeErrorResponse(error);
                    break;
                }

                m_parser.continueParsing();

                m_status = header;
                /* fall through */

            case header:
                if (m_parser.isHeaderComplete() == false)
                    return;

                if (m_parser.isBadRequest())
                {
                    m_handler->makeErrorResponse(400);
                    break;
                }
                if (int error = m_handler->processHeaders())
                {
                    m_handler->makeErrorResponse(error);
                    break;
                }

                if (m_request->timeout > 0)
                    m_timeoutDuration = Duration::seconds(m_request->timeout);
                
                m_parser.continueParsing();

                m_handler->makeResponse();

                if (m_handler->needBody() == false)
                    break;

                m_status = body;
                /* fall through */
                
            case body:
                if (m_parser.isBodyComplete() == false)
                    return;

                if (m_parser.isBadRequest())
                {
                    m_handler->makeErrorResponse(400);
                    break;
                }
            }

            m_handler->runTasks(m_handler);

            if (m_parser.isBadRequest() || m_handler->shouldEndConnection())
                break;
            
            m_request = new HTTPRequest();
            m_handler = new RequestHandler(m_request, m_clientSocket);
            m_status = requestLine;
            m_parser.nextRequest(m_request);
        }
    }

    IOManager::shared().eraseReadTask(this);
}

}