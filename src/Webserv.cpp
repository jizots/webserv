/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/13 15:52:48 by tchoquet          #+#    #+#             */
/*   Updated: 2024/02/25 14:10:42 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

#include <set>
#include <sys/select.h>

#include "IOManager.hpp"
#include "ClientSocket.hpp"

namespace webserv
{

Webserv::Webserv(const std::vector<ServerConfig>& configs) : m_running(false)
{
    typedef std::vector<ServerConfig>::const_iterator conf_consIte;

    for (conf_consIte curr = configs.begin(); curr != configs.end(); ++curr)
    {
        SharedPtr<Server> newServer = new Server(*curr);
        for (std::vector<int>::const_iterator lisCurr = curr->listens.begin(); lisCurr != curr->listens.end(); ++lisCurr)
            IOManager::shared().addServerToMasterSocket(*lisCurr, newServer);
    }

    IOManager::shared().insertReadTask(new StdinReadTask(*this));
}

void Webserv::run()
{
    m_running = true;
    while(m_running)
    {
        std::set<MasterSocketPtr> masterSockets;
        std::set<IReadTaskPtr> readTasks;
        std::set<IWriteTaskPtr> writeTasks;

        IOManager::shared().selectIOs(masterSockets, readTasks, writeTasks);

        for (std::set<MasterSocketPtr>::iterator curr = masterSockets.begin(); curr != masterSockets.end(); ++curr)
            IOManager::shared().insertReadTask(new ClientSocketReadTask(curr->acceptNewClient()));

        for (std::set<IReadTaskPtr>::iterator curr = readTasks.begin(); curr != readTasks.end(); ++curr)
            curr->read();

        for (std::set<IWriteTaskPtr>::iterator curr = writeTasks.begin(); curr != writeTasks.end(); ++curr)
            curr->write();
        
    }
}

} // namespace webserv