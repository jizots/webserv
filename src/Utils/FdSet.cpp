/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FdSet.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/30 17:31:24 by tchoquet          #+#    #+#             */
/*   Updated: 2024/05/06 12:18:25 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/FdSet.hpp"

#include <algorithm>

namespace webserv
{

FdSet::FdSet() : m_biggestFd(-1)
{
    FD_ZERO(&m_fdSet);
}

void FdSet::insert(const FileDescriptor& fd)
{
    FD_SET(fd.rawFd(), &m_fdSet);
    m_biggestFd = std::max(m_biggestFd, fd.rawFd());
}

bool FdSet::contain(const FileDescriptor& fd)
{
    return FD_ISSET(fd.rawFd(), &m_fdSet);
}

int select(FdSet& readFds, FdSet& writeFds, long usecTimeout)
{
    struct timeval timeVal;

    timeVal.tv_sec = 0;
    timeVal.tv_usec = (int)usecTimeout;

    return select(std::max(readFds.m_biggestFd, writeFds.m_biggestFd) + 1, &readFds.m_fdSet, &writeFds.m_fdSet, NULL, &timeVal);;
}

}