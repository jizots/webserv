/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileReadTask.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:08:53 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/26 16:24:04 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/IOTask/ReadTask/FileReadTask.hpp"

#include "IO/IOManager.hpp"

namespace webserv
{

FileReadTask::FileReadTask(const StaticFileResourcePtr& resource, const HTTPResponsePtr& response, const RequestHandlerPtr& handler)
    : m_resource(resource), m_response(response), m_handler(handler), m_idx(0)
{
    m_response->body.resize(m_resource->contentLength());
}

void FileReadTask::read()
{
    updateTimestamp();

    ssize_t readLen = webserv::read(fd(), m_response->body.data() + m_idx, m_resource->contentLength() - m_idx);

    if (readLen < 0)
    {
        log << "Error while reading file \"" << m_resource->path() << "\" (fd: " << fd() << "): " << std::strerror(errno) << '\n';
        m_handler->makeErrorResponse(500);
    }

    else if (readLen == 0)
    {
        log << "EOF received from file \"" << m_resource->path() << "\" (fd: " << fd() << ")\n";
        if (m_idx < m_resource->contentLength())
            m_handler->makeErrorResponse(500);
        else
            m_response->isComplete = true;
    }

    else
    {
        log << readLen << " bytes read from file \"" << m_resource->path() << "\" (fd: " << fd() << ")\n";
        m_idx += readLen;
        if (m_idx < m_resource->contentLength())
            return;
        m_response->isComplete = true;
    }

    m_handler->runTasks(m_handler);
    IOManager::shared().eraseReadTask(this);
}

}