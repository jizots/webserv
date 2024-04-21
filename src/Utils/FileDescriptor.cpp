/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileDescriptor.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/30 16:34:29 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/18 14:43:37 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/FileDescriptor.hpp"

#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstdarg>

#include "Utils/Macros.hpp"
#include "Utils/Logger.hpp"

namespace webserv
{

FileDescriptor::FileDescriptor() : m_fd(-1), m_refCount(NULL)
{
}

FileDescriptor::FileDescriptor(const FileDescriptor& cp) : m_fd(cp.m_fd), m_refCount(cp.m_refCount)
{
    if (m_refCount != NULL)
        *m_refCount += 1;
}

FileDescriptor::FileDescriptor(int fd, bool nonBlock) : m_fd(fd < 0 ? -1 : fd), m_refCount(fd < 0 ? NULL : new uint32(1))
{
    if (*this && nonBlock)
        ::fcntl(m_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
}

FileDescriptor::FileDescriptor(int fd, uint32* refCount, bool nonBlock) : m_fd(fd < 0 ? -1 : fd), m_refCount(fd < 0 ? NULL : refCount)
{
    if (*this && nonBlock)
        ::fcntl(m_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
}

const FileDescriptor& FileDescriptor::stdin()
{
    static FileDescriptor stdin = FileDescriptor(STDIN_FILENO, NULL, false);

    return stdin;
}

const FileDescriptor& FileDescriptor::stdout()
{
    static FileDescriptor stdout = FileDescriptor(STDOUT_FILENO, NULL, false);

    return stdout;
}

FileDescriptor::~FileDescriptor()
{
    close(*this);
}

FileDescriptor& FileDescriptor::operator = (const FileDescriptor& cp)
{
    if (this != &cp)
    {
        close(*this);
        m_fd = cp.m_fd;
        m_refCount = cp.m_refCount;
        if (m_refCount != NULL)
            *m_refCount += 1;
    }
    return *this;
}

std::ostream& operator << (std::ostream& os, const FileDescriptor& fd)
{
    os << (int)fd.m_fd;
    return os;    
}

int pipeWNB(FileDescriptor* fds)
{
    int fildes[2];

    int res = ::pipe(fildes);
    if (res <= 0)
    {
        fds[READ_END]  = FileDescriptor(fildes[READ_END], false);
        fds[WRITE_END] = FileDescriptor(fildes[WRITE_END], true);
    }
    return res;
}

int pipeRNB(FileDescriptor* fds)
{
    int fildes[2];

    int res = ::pipe(fildes);
    if (res <= 0)
    {
        fds[READ_END]  = FileDescriptor(fildes[READ_END], true);
        fds[WRITE_END] = FileDescriptor(fildes[WRITE_END], false);
    }
    return res;
}

FileDescriptor dup2(const FileDescriptor& fd, const FileDescriptor& fd2)
{
    int fildes = ::dup2(fd.m_fd, fd2.m_fd);

    if (fildes < 0 || fildes != fd2.m_fd)
        return FileDescriptor();

    return fd2;
}

int close(FileDescriptor& fd)
{
    int ret = 0;

    if (fd.m_refCount == NULL)
        return ret;

    *fd.m_refCount = *fd.m_refCount > 0 ? *fd.m_refCount - 1 : 0;

    if (*fd.m_refCount == 0)
    {
        log << "closing fd " << fd.m_fd << '\n';
        int ret = ::close(fd.m_fd);
        if (ret < 0)
            return ret;
        delete fd.m_refCount;
    }

    fd.m_fd = -1;
    fd.m_refCount = NULL;
    return ret;
}

}