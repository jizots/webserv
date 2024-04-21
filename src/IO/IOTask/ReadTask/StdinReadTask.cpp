/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StdinReadTask.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:12:27 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/08 23:24:17 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IO/IOTask/ReadTask/StdinReadTask.hpp"

#include "IO/IOManager.hpp"

namespace webserv
{

StdinReadTask::StdinReadTask(std::string& cmd) : m_cmd(cmd)
{
    std::cout << "webserv > " << std::flush;
}

void StdinReadTask::read()
{
    for (;;)
    {
        Byte c;
        ssize_t readLen = webserv::read(fd(), &c, 1);
        if (readLen == 1)
        {
            if (c == '\n')
                break;
            m_cmd.push_back(c);
        }
        else if (readLen < 0)
        {
            m_cmd.clear();
            return;
        }
        else
        {
            m_cmd = "quit";
            break;
        }             
    }

    if (m_cmd.empty())
        m_cmd = "none";

    IOManager::shared().eraseReadTask(this);
}

}