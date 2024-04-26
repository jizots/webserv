/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileWriteTask.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/21 17:32:57 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/26 16:18:22 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/IOTask/WriteTask/FileWriteTask.hpp"

#include "IO/IOManager.hpp"

namespace webserv
{

FileWriteTask::FileWriteTask(const NoSuchFileResourcePtr& resource, const HTTPRequestPtr& request, const RequestHandlerPtr& handler)
    : m_resource(resource), m_request(request), m_handler(handler), m_idx(0)
{
}

void FileWriteTask::write()
{
    updateTimestamp();
    
    ssize_t writeLen = webserv::write(fd(), m_request->body.data() + m_idx, m_request->body.size() - m_idx);
    
    if (writeLen < 0)
    {
        log << "Error while writing to file " << m_resource->path() << " (fd: " << fd() << "): " << std::strerror(errno) << '\n';
        m_handler->makeErrorResponse(500);
    }

    else if (writeLen == 0)
    {
        log << "No data write to file " << m_resource->path() << " (fd: " << fd() << ")\n";
        if (m_idx < m_request->body.size())
            m_handler->makeErrorResponse(500);
        else
            m_handler->makeErrorResponse(201);
    }

    else
    {
        log << writeLen << " bytes write to file " << m_resource->path() << " (fd: " << fd() << ")\n";
        m_idx += writeLen;
        if (m_idx < m_request->body.size())
            return;
        m_handler->makeErrorResponse(201);
    }

    m_handler->runTasks(m_handler);
    IOManager::shared().eraseWriteTask(this);
}

}