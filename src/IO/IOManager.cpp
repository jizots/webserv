/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IOManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/18 15:16:58 by tchoquet          #+#    #+#             */
/*   Updated: 2024/02/19 16:18:26 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/IOManager.hpp"

#include <sys/select.h>
#include <cstring>

namespace webserv
{

IOManager* IOManager::s_instance = NULL;

void IOManager::init()
{
    if (s_instance != NULL)
        return;
    s_instance = new IOManager;
}

void IOManager::terminate()
{
    delete s_instance;
}

void IOManager::selectIOs(std::set<MasterSocketPtr>& masterSockets, std::set<IReadTaskPtr>& readTasks,std::set<IWriteTaskPtr>& writeTasks)
{
    fd_set readableFdSet;
    fd_set writableFdSet;

    int biggestFd = 0;

    FD_ZERO(&readableFdSet);
    FD_ZERO(&writableFdSet);

    masterSockets.clear();
    readTasks.clear();
    writeTasks.clear();

    for (std::map<uint16, MasterSocketPtr>::iterator curr = m_masterSockets.begin(); curr != m_masterSockets.end(); ++curr)
    {
        FD_SET(curr->second->fileDescriptor(), &readableFdSet);
        biggestFd = std::max(biggestFd, curr->second->fileDescriptor());
    }

    for (std::set<IReadTaskPtr>::iterator curr = m_readTasks.begin(); curr != m_readTasks.end(); ++curr)
    {
        FD_SET((*curr)->fd(), &readableFdSet);
        biggestFd = std::max(biggestFd, (*curr)->fd());
    }

    for (std::set<IWriteTaskPtr>::iterator curr = m_writeTasks.begin(); curr != m_writeTasks.end(); ++curr)
    {
        FD_SET((*curr)->fd(), &writableFdSet);
        biggestFd = std::max(biggestFd, (*curr)->fd());
    }

    if (select(biggestFd + 1, &readableFdSet, &writableFdSet, NULL, NULL) < 0)
        throw std::runtime_error("select: " + std::string(std::strerror(errno)));

    for (std::map<uint16, MasterSocketPtr>::iterator curr = m_masterSockets.begin(); curr != m_masterSockets.end(); ++curr)
    {
        if (FD_ISSET(curr->second->fileDescriptor(), &readableFdSet))
            masterSockets.insert(curr->second);
    }

    for (std::set<IReadTaskPtr>::iterator curr = m_readTasks.begin(); curr != m_readTasks.end(); ++curr)
    {
        if (FD_ISSET((*curr)->fd(), &readableFdSet))
            readTasks.insert(*curr);
    }

    for (std::set<IWriteTaskPtr>::iterator curr = m_writeTasks.begin(); curr != m_writeTasks.end(); ++curr)
    {
        if (FD_ISSET((*curr)->fd(), &writableFdSet))
            writeTasks.insert(*curr);
    }
}

void IOManager::eraseReadTask(IReadTask* task)
{
    for (std::set<IReadTaskPtr>::iterator curr = m_readTasks.begin(); curr != m_readTasks.end();)
    {
        if ((*curr) == task)
        {
            std::set<IReadTaskPtr>::iterator pos = curr;
            ++curr;
            m_readTasks.erase(pos);
        }
        else
            ++curr;
    }
}

void IOManager::eraseWriteTask(IWriteTask* task)
{
    for (std::set<IWriteTaskPtr>::iterator curr = m_writeTasks.begin(); curr != m_writeTasks.end();)
    {
        if ((*curr) == task)
        {
            std::set<IWriteTaskPtr>::iterator pos = curr;
            ++curr;
            m_writeTasks.erase(pos);
        }
        else
            ++curr;
    }
}

}