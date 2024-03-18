/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StdinReadTask.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:11:29 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/06 17:29:42 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STDINREADTASK_HPP
# define STDINREADTASK_HPP

#include "IO/IOTask.hpp"

namespace webserv
{

class StdinReadTask : public IReadTask
{
public:
    StdinReadTask();
    int fd() /*override*/;
    void read() /*override*/;
};

}

#endif // STDINREADTASK_HPP