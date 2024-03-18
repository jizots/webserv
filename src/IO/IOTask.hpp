/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IOTask.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/19 16:34:02 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/06 17:29:21 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IOTASK_HPP
# define IOTASK_HPP

#include "Utils/Utils.hpp"

namespace webserv
{

class IOTask
{
public:
    virtual int fd() = 0;
};

class IReadTask;
typedef SharedPtr<IReadTask> IReadTaskPtr;
class IReadTask : public IOTask
{
public:
    virtual void read() = 0;
    inline virtual ~IReadTask() {}
};

class IWriteTask;
typedef SharedPtr<IWriteTask> IWriteTaskPtr;
class IWriteTask : public IOTask
{
public:
    virtual void write() = 0;
    inline virtual ~IWriteTask() {}
};

}

#endif // IOTASK_HPP