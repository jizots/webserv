/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StdinReadTask.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:11:29 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/06 18:31:34 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STDINREADTASK_HPP
# define STDINREADTASK_HPP

#include "IO/IOTask/IOTask.hpp"

#include <string>

#include "Utils/Utils.hpp"

namespace webserv
{

class StdinReadTask : public IReadTask
{
public:
    StdinReadTask(std::string& cmd);

    inline const FileDescriptor& fd() { return FileDescriptor::stdin(); };

    void read() /*override*/;

private:
    std::string& m_cmd;
};

}

#endif // STDINREADTASK_HPP