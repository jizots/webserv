/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/10 13:34:28 by tchoquet          #+#    #+#             */
/*   Updated: 2024/05/06 10:32:57 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/Logger.hpp"
#include "Utils/Functions.hpp"

#include "Parser/ConfigParser/ConfigParser.hpp"

#include "IO/IOManager.hpp"
#include "IO/IOTask/ReadTask/StdinReadTask.hpp"
#include "IO/IOTask/ReadTask/ClientSocketReadTask.hpp"

using namespace webserv;

#ifndef NDEBUG
    #ifdef __APPLE__
        #include <unistd.h>
        #include <cstdlib>

        pid_t pid = getpid();
        
        __attribute__((destructor))
        static void	destructor(void)
        {
            if (getpid() == pid)
                std::system(std::string("leaks -q " + to_string(getpid())).c_str());
        }
    #endif
#endif // NDEBUG

int main(int argc, const char* argv[])
{
    int returnCode = 0;
    
    try
    {
        IOManager::init();

        std::vector<ServerConfig> configs = parseServerConfig(argc, argv);
        Logger::init(configs[0].error_log);
        IOManager::shared().loadConfigs(configs);
        goto loop;

reload:
        try
        {
            std::vector<ServerConfig> newConfig = parseServerConfig(argc, argv);
            Logger::terminate();
            Logger::init(configs[0].error_log);
            IOManager::shared().loadConfigs(newConfig); // TODO Chek what happen if this fail (bind: adress used)
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }

loop:
        std::string cmd = "";
        IOManager::shared().insertReadTask(new StdinReadTask(cmd));
        while(cmd.empty())
        {
            std::set<MasterSocketPtr> masterSockets;
            std::set<IReadTaskPtr> readTasks;
            std::set<IWriteTaskPtr> writeTasks;

            IOManager::shared().selectIOs(masterSockets, readTasks, writeTasks);

            for (std::set<MasterSocketPtr>::iterator curr = masterSockets.begin(); curr != masterSockets.end(); ++curr)
            {
                if (ClientSocketPtr clientSocket = (*curr)->acceptNewClient(*curr))
                    IOManager::shared().insertReadTask(new ClientSocketReadTask(clientSocket));
            }

            for (std::set<IReadTaskPtr>::iterator curr = readTasks.begin(); curr != readTasks.end(); ++curr)
                (*curr)->read();

            for (std::set<IWriteTaskPtr>::iterator curr = writeTasks.begin(); curr != writeTasks.end(); ++curr)
                (*curr)->write();
        }

        if (cmd == "reload")
            goto reload;

        if (cmd != "quit")
            goto loop;
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        returnCode = 1;
    }

    IOManager::terminate();
    Logger::terminate();

    return returnCode;
}
