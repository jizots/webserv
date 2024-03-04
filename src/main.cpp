/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/10 13:34:28 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/02 00:19:27 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/Utils.hpp"
#include "ConfigParser/ConfigParser.hpp"
#include "IOManager.hpp"
#include "Webserv.hpp"

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

        Webserv(configs).run();

        webserv::IOManager::terminate();
        webserv::Logger::terminate();
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}
