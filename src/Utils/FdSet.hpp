/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FdSet.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/30 17:31:14 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/20 09:01:35 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FDSET_HPP
# define FDSET_HPP

#include <sys/select.h>
#include <set>

#include "Utils/FileDescriptor.hpp"

namespace webserv
{

class FdSet
{
public:
    FdSet();
    
    void insert(const FileDescriptor& fd);
    bool contain(const FileDescriptor& fd);

    friend int select(FdSet& readFds, FdSet& writeFds, long usecTimeout);

private:
    fd_set m_fdSet;
    int m_biggestFd;
};

}

#endif // FDSET_HPP