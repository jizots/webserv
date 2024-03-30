/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileWriteTask.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/29 19:20:20 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/30 13:09:07 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/FileWriteTask.hpp"

#include <unistd.h>

#include "IO/IOManager.hpp"
#include "IO/ClientSocketWriteTask.hpp"

namespace webserv
{

FileWriteTask::FileWriteTask(const std::vector<NewFileResourcePtr>& newFileResources,
                             const HTTPRequestPtr& request,
                             const HTTPResponsePtr& response,
                             const ClientSocketPtr& clientSocket)

                          : m_newFileResources(newFileResources),
                            m_request(request),
                            m_response(response),
                            m_clientSocket(clientSocket),
                            m_newFileResourceCurr(m_newFileResources.begin()),
                            m_multipartDataCurr(m_request->m_multipartFormDatas.begin()),
                            m_idx(0)
{
}

int FileWriteTask::fd() /*override*/
{
    return (*m_newFileResourceCurr)->writeFd();
}

void FileWriteTask::write() /*override*/
{
    updateTimestamp();
    
    uint32 writeLen = m_multipartDataCurr->lenData - m_idx > BUFFER_SIZE ? BUFFER_SIZE : m_multipartDataCurr->lenData - m_idx;

    if (::write(fd(), &m_request->body[m_multipartDataCurr->dataStartPos + m_idx], writeLen) != writeLen)
        throw std::runtime_error("write: " + std::string(std::strerror(errno)));
    
    m_idx += writeLen;
    if (m_idx == m_multipartDataCurr->lenData)
    {
        log << m_multipartDataCurr->lenData << " bytes write to file " << (*m_newFileResourceCurr)->path() << " (fd: " << fd() << ")\n";

        ++m_newFileResourceCurr;
        ++m_multipartDataCurr;
        m_idx = 0;

        if (m_newFileResourceCurr == m_newFileResources.end() || m_multipartDataCurr == m_request->m_multipartFormDatas.end())
        {
            IOManager::shared().eraseWriteTask(this);

            m_response->isComplete = true;
            if (m_clientSocket->nextResponse() == m_response)
                IOManager::shared().insertWriteTask(new ClientSocketWriteTask(m_clientSocket, m_clientSocket->nextResponse()));

        }
    }
}

}