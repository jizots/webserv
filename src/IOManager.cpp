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

#include "IOManager.hpp"

#include <sys/select.h>

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

void IOManager::addServerToMasterSocket(int port, const ServerPtr& serv)
{
    std::map<uint32, MasterSocketPtr>::iterator it = m_masterSockets.find(port);
    if (it == m_masterSockets.end())
        it = m_masterSockets.insert(std::make_pair(port, new MasterSocket(port))).first;
    it->second->addServer(serv);
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

    for (std::map<uint32, MasterSocketPtr>::iterator curr = m_masterSockets.begin(); curr != m_masterSockets.end(); ++curr)
    {
        FD_SET(curr->second->fileDescriptor(), &readableFdSet);
        biggestFd = max(biggestFd, curr->second->fileDescriptor());
    }

    for (std::set<IReadTaskPtr>::iterator curr = m_readTasks.begin(); curr != m_readTasks.end(); ++curr)
    {
        FD_SET((*curr)->fd(), &readableFdSet);
        biggestFd = max(biggestFd, (*curr)->fd());
    }

    for (std::set<IWriteTaskPtr>::iterator curr = m_writeTasks.begin(); curr != m_writeTasks.end(); ++curr)
    {
        FD_SET((*curr)->fd(), &writableFdSet);
        biggestFd = max(biggestFd, (*curr)->fd());
    }

    if (select(biggestFd + 1, &readableFdSet, &writableFdSet, NULL, NULL) < 0)
        throw std::runtime_error("select: " + std::string(std::strerror(errno)));

    for (std::map<uint32, MasterSocketPtr>::iterator curr = m_masterSockets.begin(); curr != m_masterSockets.end(); ++curr)
    {
        if (FD_ISSET(curr->second->fileDescriptor(), &readableFdSet))
            masterSockets.insert(curr->second);
    }

    for (std::set<IReadTaskPtr>::iterator curr = m_readTasks.begin(); curr != m_readTasks.end();)
    {
        if (FD_ISSET((*curr)->fd(), &readableFdSet))
        {
            readTasks.insert(*curr);
            std::set<IReadTaskPtr>::iterator pos = curr;
            ++curr;
            m_readTasks.erase(pos);
        }
        else
            ++curr;
    }

    for (std::set<IWriteTaskPtr>::iterator curr = m_writeTasks.begin(); curr != m_writeTasks.end();)
    {
        if (FD_ISSET((*curr)->fd(), &writableFdSet))
        {
            writeTasks.insert(*curr);
            std::set<IWriteTaskPtr>::iterator pos = curr;
            ++curr;
            m_writeTasks.erase(pos);
        }
        else
            ++curr;
    }
}

}