/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sotanaka <sotanaka@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/10 13:34:28 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/21 18:47:12 by sotanaka         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/Utils.hpp"

#include "ConfigParser/ConfigParser.hpp"

#include "IO/IOManager.hpp"
#include "IO/StdinReadTask.hpp"
#include "IO/ClientSocketReadTask.hpp"

using namespace webserv;

#ifndef NDEBUG
    #ifdef __APPLE__
        #include <unistd.h>
        #include <cstdlib>
        
        __attribute__((destructor))
        static void	destructor(void)
        {
            std::system(std::string("leaks -q " + to_string(getpid())).c_str());
        }
    #endif
#endif // NDEBUG

int main(int argc, const char* argv[])
{
    try
    {
        std::vector<ServerConfig> configs = parseServerConfig(argc, argv);

        webserv::Logger::init(configs[0].error_log);
        webserv::IOManager::init();

        for (std::vector<ServerConfig>::const_iterator confCurr = configs.begin(); confCurr != configs.end(); ++confCurr)
        {
            for (std::vector<int>::const_iterator lisCurr = confCurr->listens.begin(); lisCurr != confCurr->listens.end(); ++lisCurr)
            {
                std::map<uint16, MasterSocketPtr>::iterator it = IOManager::shared().masterSockets().find((uint16)*lisCurr);
                if (it == IOManager::shared().masterSockets().end())
                    it = IOManager::shared().masterSockets().insert(std::make_pair(*lisCurr, new MasterSocket(*lisCurr))).first;
                it->second->addServerConfig(*confCurr);
            }
        }

        IOManager::shared().insertReadTask(new StdinReadTask());

        while(1)
        {
            std::set<MasterSocketPtr> masterSockets;
            std::set<IReadTaskPtr> readTasks;
            std::set<IWriteTaskPtr> writeTasks;

            IOManager::shared().selectIOs(masterSockets, readTasks, writeTasks);

            for (std::set<MasterSocketPtr>::iterator curr = masterSockets.begin(); curr != masterSockets.end(); ++curr)
                IOManager::shared().insertReadTask(new ClientSocketReadTask((*curr)->acceptNewClient(*curr)));

            for (std::set<IReadTaskPtr>::iterator curr = readTasks.begin(); curr != readTasks.end(); ++curr)
                (*curr)->read();

            for (std::set<IWriteTaskPtr>::iterator curr = writeTasks.begin(); curr != writeTasks.end(); ++curr)
                (*curr)->write();
            
        }

        webserv::IOManager::terminate();
        webserv::Logger::terminate();
    }
    catch(const ConfigException& e)
    {
        std::cout << e.what() << std::endl;
        return 2;
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }
    catch(int code)
    {
        webserv::IOManager::terminate();
        webserv::Logger::terminate();
        return code;
    }
    return 0;
}
