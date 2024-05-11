/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IOManager.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/18 15:16:58 by tchoquet          #+#    #+#             */
/*   Updated: 2024/05/07 20:44:36 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/IOManager.hpp"

#include <cstring>

#include "IO/IOTask/ReadTask/StdinReadTask.hpp"

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
    s_instance = NULL;
}

void IOManager::loadConfigs(const std::vector<ServerConfig>& configs)
{
    m_readTasks.clear();
    m_writeTasks.clear();
    
    for (std::map<uint16, MasterSocketPtr>::iterator it = m_masterSockets.begin(); it != m_masterSockets.end(); ++it)
        it->second->clearConfigs();

    for (std::vector<ServerConfig>::const_iterator confCurr = configs.begin(); confCurr != configs.end(); ++confCurr)
    {
        for (std::vector<uint16>::const_iterator lisCurr = confCurr->listens.begin(); lisCurr != confCurr->listens.end(); ++lisCurr)
        {
            std::map<uint16, MasterSocketPtr>::iterator it = m_masterSockets.find((uint16)*lisCurr);
            if (it == m_masterSockets.end())
                it = m_masterSockets.insert(std::make_pair(*lisCurr, new MasterSocket(*lisCurr))).first;
            it->second->addServerConfig(*confCurr);
        }
    }

    for (std::map<uint16, MasterSocketPtr>::iterator curr = m_masterSockets.begin(); curr != m_masterSockets.end();)
    {
        if (curr->second->hasConfig() == false)
        {
            std::map<uint16, MasterSocketPtr>::iterator pos = curr;
            ++curr;
            m_masterSockets.erase(pos);
        }
        else
            ++curr;
    }
}

void IOManager::selectIOs(std::set<MasterSocketPtr>& masterSockets, std::set<IReadTaskPtr>& readTasks,std::set<IWriteTaskPtr>& writeTasks)
{
    FdSet readableFdSet;
    FdSet writableFdSet;

    masterSockets.clear();
    readTasks.clear();
    writeTasks.clear();

    for (std::map<uint16, MasterSocketPtr>::iterator curr = m_masterSockets.begin(); curr != m_masterSockets.end(); ++curr)
        readableFdSet.insert(curr->second->fileDescriptor());

    for (std::set<IReadTaskPtr>::iterator curr = m_readTasks.begin(); curr != m_readTasks.end(); ++curr)
        readableFdSet.insert((*curr)->fd());

    for (std::set<IWriteTaskPtr>::iterator curr = m_writeTasks.begin(); curr != m_writeTasks.end(); ++curr)
        writableFdSet.insert((*curr)->fd());

    if (select(readableFdSet, writableFdSet, 250000) < 0)
        throw std::runtime_error("select: " + std::string(std::strerror(errno)));

    for (std::map<uint16, MasterSocketPtr>::iterator curr = m_masterSockets.begin(); curr != m_masterSockets.end(); ++curr)
    {
        if (readableFdSet.contain(curr->second->fileDescriptor()))
            masterSockets.insert(curr->second);
    }

    for (std::set<IReadTaskPtr>::iterator curr = m_readTasks.begin(); curr != m_readTasks.end();)
    {
        if (readableFdSet.contain((*curr)->fd()))
        {
            readTasks.insert(*curr);
            ++curr;
        }
        else if (SharedPtr<StdinReadTask> stdinReadTask = (*curr).dynamicCast<StdinReadTask>())
            ++curr;
        else if ((*curr)->isTimeout())
        {
            log << "task timeout, deleting\n";
            std::set<IReadTaskPtr>::iterator pos = curr;
            ++curr;
            m_readTasks.erase(pos);
        }
        else
            ++curr;
    }

    for (std::set<IWriteTaskPtr>::iterator curr = m_writeTasks.begin(); curr != m_writeTasks.end();)
    {
        if (writableFdSet.contain((*curr)->fd()))
        {
            writeTasks.insert(*curr);
            ++curr;
        }
        else if ((*curr)->isTimeout())
        {
            log << "task timeout, deleting\n";
            std::set<IWriteTaskPtr>::iterator pos = curr;
            ++curr;
            m_writeTasks.erase(pos);
        }
        else
            ++curr;
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