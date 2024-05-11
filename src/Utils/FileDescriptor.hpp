/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileDescriptor.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/30 16:30:22 by tchoquet          #+#    #+#             */
/*   Updated: 2024/05/08 00:11:45 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILEDESCRIPTOR_HPP
# define FILEDESCRIPTOR_HPP

#include <cstddef>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/fcntl.h>

#include "Utils/Types.hpp"

#undef stdin
#undef stdout

#define READ_END 0
#define WRITE_END 1

namespace webserv
{

class FileDescriptor
{

private:
    friend FileDescriptor open(const std::string& path, int flags);
    friend FileDescriptor open(const std::string& path, int flags, int options);
    friend FileDescriptor socket(int domain, int type, int protocol);
    friend FileDescriptor accept(const FileDescriptor& fd, struct sockaddr* address, socklen_t* address_len);
    friend FileDescriptor dup(const FileDescriptor& fd);
    friend int pipeWNB(FileDescriptor* fds);
    friend int pipeRNB(FileDescriptor* fds);
    friend FileDescriptor dup2(const FileDescriptor& fd, const FileDescriptor& fd2);
    friend int bind(const FileDescriptor& fd, const struct sockaddr* address, socklen_t address_len);
    friend int listen(const FileDescriptor& fd, int backlog);
    friend ssize_t recv(const FileDescriptor& fd, void* buffer, size_t length, int flags);
    friend ssize_t send(const FileDescriptor& fd, void* buffer, size_t length, int flags);
    friend ssize_t read(const FileDescriptor& fd, void* buf, size_t nbyte);
    friend ssize_t write(const FileDescriptor& fd, const void* buf, size_t nbyte);
    friend void write(const FileDescriptor& fd, const std::string& str);
    friend int close(FileDescriptor& fd);
    friend int setsockopt(const FileDescriptor& fd, int level, int option_name, const void *option_value, socklen_t option_len);


public:
    FileDescriptor();
    FileDescriptor(const FileDescriptor&);

    static const FileDescriptor& stdin();
    static const FileDescriptor& stdout();

    inline int rawFd() const { return m_fd; }

    ~FileDescriptor();

private:
    FileDescriptor(int fd, bool nonBlock = true);
    FileDescriptor(int fd, uint32* refCount, bool nonBlock);

    int m_fd;
    uint32* m_refCount;

public:
    FileDescriptor& operator = (const FileDescriptor&);

    inline bool operator == (const FileDescriptor& rhs) const { return m_fd == rhs.m_fd; }
    inline bool operator != (const FileDescriptor& rhs) const { return m_fd != rhs.m_fd; }
    inline bool operator  < (const FileDescriptor& rhs) const { return m_fd  < rhs.m_fd; }

    inline operator bool () const { return m_fd >= 0; }

    friend std::ostream& operator << (std::ostream& os, const FileDescriptor& fd);
};

int pipeWNB(FileDescriptor* fds);
int pipeRNB(FileDescriptor* fds);
FileDescriptor dup2  (const FileDescriptor& fd, const FileDescriptor& fd2);
int close(FileDescriptor& fd);

inline FileDescriptor open  (const std::string& path, int flags)                                         { return ::open(path.c_str(), flags);             }
inline FileDescriptor open  (const std::string& path, int flags, int options)                            { return ::open(path.c_str(), flags, options);    }
inline FileDescriptor socket(int domain, int type, int protocol)                                         { return ::socket(domain, type, protocol);        }
inline FileDescriptor accept(const FileDescriptor& fd, struct sockaddr* address, socklen_t* address_len) { return ::accept(fd.m_fd, address, address_len); }
inline FileDescriptor dup   (const FileDescriptor& fd)                                                   { return ::dup(fd.m_fd);                          }

inline     int bind  (const FileDescriptor& fd, const struct sockaddr *address, socklen_t address_len) { return ::bind(fd.m_fd, address, address_len);                   }
inline     int listen(const FileDescriptor& fd, int backlog)                                           { return ::listen(fd.m_fd, backlog);                              }
inline ssize_t recv  (const FileDescriptor& fd, void *buffer, size_t length, int flags)                { return ::recv(fd.m_fd, buffer, length, flags);                  }
inline ssize_t send  (const FileDescriptor& fd, void *buffer, size_t length, int flags)                { return ::send(fd.m_fd, buffer, length, flags);                  }
inline ssize_t read  (const FileDescriptor& fd, void* buf, size_t nbyte)                               { return ::read(fd.m_fd, buf, nbyte);                             }
inline ssize_t write (const FileDescriptor& fd, const void* buf, size_t nbyte)                         { return ::write(fd.m_fd, buf, nbyte);                            }
inline    void write (const FileDescriptor& fd, const std::string& str)                                { ssize_t n = ::write(fd.m_fd, str.c_str(), str.size()); (void)n; }

inline int setsockopt(const FileDescriptor& fd, int level, int option_name, const void *option_value, socklen_t option_len) { return ::setsockopt(fd.m_fd, level, option_name, option_value, option_len); }

}

#endif // FILEDESCRIPTOR_HPP