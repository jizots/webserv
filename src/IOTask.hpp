/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IOTask.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/19 16:34:02 by tchoquet          #+#    #+#             */
/*   Updated: 2024/02/25 13:06:07 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IOTASK_HPP
# define IOTASK_HPP

#include <unistd.h>

#include "Utils/Utils.hpp"
#include "ClientSocket.hpp" 
#include "HTTP/HTTPRequestParser.hpp"
#include "Webserv.hpp"

namespace webserv
{

class IOTask
{
public:
    virtual int fd() = 0;
};

class IReadTaskPtr;
class IReadTask : public IOTask
{
public:
    virtual void read(const IReadTaskPtr&) = 0;
    inline virtual ~IReadTask() {}
};

class IReadTaskPtr : public SharedPtr<IReadTask>
{
public:
    inline IReadTaskPtr()                                            : SharedPtr<IReadTask>()    {}
    inline IReadTaskPtr(const IReadTaskPtr& cp)                      : SharedPtr<IReadTask>(cp)  {}
    inline IReadTaskPtr(IReadTask* ptr)                              : SharedPtr<IReadTask>(ptr) {}

    inline void read() const { (*m_pointer)->read(*this); }
};

class ClientSocketReadTask : public IReadTask
{
public:
    inline ClientSocketReadTask(const ClientSocketPtr& clientSocket) : m_clientSocket(clientSocket) {}

    inline int fd() /*override*/ { return m_clientSocket->fileDescriptor(); }
    void read(const IReadTaskPtr&) /*override*/;
    inline ~ClientSocketReadTask() /*override*/ {}

private:
    ClientSocketPtr m_clientSocket;
    HTTPRequestParser m_parser;
};

class FileReadTask : public IReadTask
{
public:
    inline FileReadTask(int fd, const ClientSocketPtr& clientSocket, const HTTPResponsePtr& response) : m_fd(fd), m_clientSocket(clientSocket), m_response(response) {}

    inline int fd() /*override*/ { return m_fd; }
    void read(const IReadTaskPtr&) /*override*/;
    inline ~FileReadTask() /*override*/ { close(m_fd); }

private:
    int m_fd;
    ClientSocketPtr m_clientSocket;
    HTTPResponsePtr m_response;
};

class StdinReadTask : public IReadTask
{
public:
    inline StdinReadTask(Webserv& webserv) : m_webserv(webserv) { std::cout << "webserv > " << std::flush; }
    inline int fd() /*override*/ { return STDIN_FILENO; }
    void read(const IReadTaskPtr&) /*override*/;
    inline ~StdinReadTask() /*override*/ { }

private:
    Webserv& m_webserv;
};

class IWriteTaskPtr;
class IWriteTask : public IOTask
{
public:
    virtual void write(const IWriteTaskPtr&) = 0;
    inline virtual ~IWriteTask() {}
};

class IWriteTaskPtr : public SharedPtr<IWriteTask>
{
public:
    inline IWriteTaskPtr()                                            : SharedPtr<IWriteTask>()    {}
    inline IWriteTaskPtr(const IWriteTaskPtr& cp)                     : SharedPtr<IWriteTask>(cp)  {}
    inline IWriteTaskPtr(IWriteTask* ptr)                             : SharedPtr<IWriteTask>(ptr) {}

    inline void write() const { (*m_pointer)->write(*this); }
};

class ClientSocketWriteTask : public IWriteTask
{
public:
    ClientSocketWriteTask(const ClientSocketPtr& clientSocket, const HTTPResponse& resp);

    inline int fd() /*override*/ { return m_clientSocket->fileDescriptor(); }
    void write(const IWriteTaskPtr&) /*override*/;
    inline std::vector<Byte>& buffer() { return m_buffer; };
    ~ClientSocketWriteTask();

private:
    ClientSocketPtr m_clientSocket;
    std::vector<Byte> m_buffer;
    uint64 m_idx;
};

}

#endif // IOTASK_HPP