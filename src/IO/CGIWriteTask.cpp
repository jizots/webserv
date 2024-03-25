/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIWriteTask.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:16:53 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/24 16:44:51 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/CGIWriteTask.hpp"

#include <unistd.h>

#include "IO/IOManager.hpp"

namespace webserv
{

CGIWriteTask::CGIWriteTask(int writeFd, const HTTPRequest& request)
    : m_writeFd(writeFd), m_idx(0)
{
    m_buffer = request.body;
}

int CGIWriteTask::fd()
{
    return m_writeFd;
}

void CGIWriteTask::write()
{
    updateTimestamp();
    
    uint32 writeLen = m_buffer.size() - m_idx > BUFFER_SIZE ? BUFFER_SIZE : m_buffer.size() - m_idx;

    if (::write(m_writeFd, &m_buffer[m_idx], writeLen) != writeLen)
        throw std::runtime_error("write: " + std::string(std::strerror(errno)));
    
    m_idx += writeLen;
    if (m_idx == m_buffer.size())
    {
        log << m_buffer.size() << " bytes send to cgi (fd: " << m_writeFd << ")\n";
        IOManager::shared().eraseWriteTask(this);
    }
}

CGIWriteTask::~CGIWriteTask()
{
    ::close(m_writeFd);
}

}