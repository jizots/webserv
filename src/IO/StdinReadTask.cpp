/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StdinReadTask.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:12:27 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/06 17:30:24 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/StdinReadTask.hpp"

#include <unistd.h>

namespace webserv
{

StdinReadTask::StdinReadTask()
{
    std::cout << "webserv > " << std::flush;
}

int StdinReadTask::fd()
{
    return STDIN_FILENO;
}

void StdinReadTask::read()
{
    std::string cmd;
    
    std::getline(std::cin, cmd);
    if (cmd == "quit" || std::cin.eof())
        throw 0;
    else
        std::cout << "webserv > " << std::flush;
}

}