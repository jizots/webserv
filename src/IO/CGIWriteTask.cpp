/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIWriteTask.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:16:53 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/25 19:28:28 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/CGIWriteTask.hpp"

#include <unistd.h>

#include "IO/IOManager.hpp"

namespace webserv
{

CGIWriteTask::CGIWriteTask(CGIProgramPtr cgiProgram, const HTTPRequestPtr& request)
    : m_cgiProgram(cgiProgram), m_request(request), m_idx(0)
{
}

int CGIWriteTask::fd()
{
    return m_cgiProgram->writeFd();
}

void CGIWriteTask::write()
{
    updateTimestamp();
    
    uint32 writeLen = m_request->body.size() - m_idx > BUFFER_SIZE ? BUFFER_SIZE : m_request->body.size() - m_idx;

    if (::write(fd(), &m_request->body[m_idx], writeLen) != writeLen)
        throw std::runtime_error("write: " + std::string(std::strerror(errno)));
    
    m_idx += writeLen;
    if (m_idx == m_request->body.size())
    {
        log << m_request->body.size() << " bytes send to cgi (fd: " << fd() << ")\n";
        IOManager::shared().eraseWriteTask(this);
    }
}

CGIWriteTask::~CGIWriteTask()
{
    m_cgiProgram->closeWriteFd();
}

}