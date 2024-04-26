/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIWriteTask.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:16:53 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/26 16:24:17 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/IOTask/WriteTask/CGIWriteTask.hpp"

#include "IO/IOManager.hpp"

namespace webserv
{

CGIWriteTask::CGIWriteTask(const FileDescriptor& fd, const HTTPRequestPtr& request, const RequestHandlerPtr& handler)
    : m_fd(fd), m_request(request), m_handler(handler), m_idx(0)
{
}

void CGIWriteTask::write()
{
    updateTimestamp();
    
    ssize_t writeLen = webserv::write(fd(), m_request->body.data() + m_idx, m_request->body.size() - m_idx);
    
    if (writeLen <= 0)
    {
        log << "Error while writing to cgi (fd: " << fd() << "): " << std::strerror(errno) << '\n';
        return;
    }
    else
    {
        log << writeLen << " bytes send to cgi (fd: " << fd() << ")\n";
        m_idx += writeLen;
        if (m_idx < m_request->body.size())
            return;
    }

    IOManager::shared().eraseWriteTask(this);
}

}