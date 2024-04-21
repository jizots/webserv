/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IOManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/18 15:14:11 by tchoquet          #+#    #+#             */
/*   Updated: 2024/02/19 16:19:30 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IOMANAGER_HPP
# define IOMANAGER_HPP

#include <map>
#include <set>

#include "Utils/Utils.hpp"
#include "Socket/MasterSocket.hpp"
#include "IO/IOTask/IOTask.hpp"

namespace webserv
{

class IOManager
{
public:
    static void init();
    static inline IOManager& shared() { return *s_instance; }
    static void terminate();

    void loadConfigs(const std::vector<ServerConfig>& configs);

    void selectIOs(std::set<MasterSocketPtr>&, std::set<IReadTaskPtr>&, std::set<IWriteTaskPtr>&);

    inline void insertReadTask(const IReadTaskPtr& task) { m_readTasks.insert(task); }
    inline void insertWriteTask(const IWriteTaskPtr& task) { m_writeTasks.insert(task); }

    inline void eraseReadTask(const IReadTaskPtr& task) { m_readTasks.erase(task); }
    inline void eraseWriteTask(const IWriteTaskPtr& task) { m_writeTasks.erase(task); }

    void eraseReadTask(IReadTask* task);
    void eraseWriteTask(IWriteTask* task);

private:
    inline  IOManager()/* = default*/ {}
    inline ~IOManager()/* = default*/ {}
    
    static IOManager* s_instance;

    std::map<uint16, MasterSocketPtr> m_masterSockets;
    std::set<IReadTaskPtr> m_readTasks;
    std::set<IWriteTaskPtr> m_writeTasks;
};

}

#endif // IOMANAGER_HPP