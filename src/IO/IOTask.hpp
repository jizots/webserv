/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IOTask.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/19 16:34:02 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/25 14:25:49 by tchoquet         ###   ########.fr       */
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
    inline IOTask(Duration timeoutDuration) : m_timestamp(Time::shared().now()), m_timeoutDuration(timeoutDuration) {}

    virtual int fd() = 0;
    inline void updateTimestamp() { m_timestamp = Time::shared().now(); }
    inline bool isTimeout() { return Time::shared().since(m_timestamp) > m_timeoutDuration; }

protected:
    Timestamp m_timestamp;
    Duration m_timeoutDuration;
};

class IReadTask;
typedef SharedPtr<IReadTask> IReadTaskPtr;
class IReadTask : public IOTask
{
public:
    inline IReadTask(Duration timeoutDuration = Duration::infinity()) : IOTask(timeoutDuration) {}

    virtual void read() = 0;
    inline virtual ~IReadTask() {}
};

class IWriteTask;
typedef SharedPtr<IWriteTask> IWriteTaskPtr;
class IWriteTask : public IOTask
{
public:
    inline IWriteTask(Duration timeoutDuration = Duration::infinity()) : IOTask(timeoutDuration) {}

    virtual void write() = 0;
    inline virtual ~IWriteTask() {}
};

}

#endif // IOTASK_HPP