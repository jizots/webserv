/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MasterSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/17 15:46:39 by tchoquet          #+#    #+#             */
/*   Updated: 2024/02/21 19:40:33 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "MasterSocket.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "ClientSocket.hpp"

namespace webserv
{

MasterSocket::MasterSocket(uint32 port) : m_fileDescriptor(socket(AF_INET, SOCK_STREAM, 0))
{
    if (m_fileDescriptor < 0)
        throw std::runtime_error("socket: " + std::string(std::strerror(errno)));
    
    sockaddr_in address = (sockaddr_in){
        .sin_family = AF_INET, .sin_port = htons(port), .sin_addr=(in_addr){.s_addr=INADDR_ANY}
    };

    if (bind(m_fileDescriptor, reinterpret_cast<const sockaddr*>(&address), sizeof(sockaddr_in)) < 0)
        throw std::runtime_error("bind: " + std::string(std::strerror(errno)));

    if (listen(m_fileDescriptor, 10) < 0) // is 10 OK ?
        throw std::runtime_error("listen: " + std::string(std::strerror(errno)));

    log << "Master socket created for port: " << port << '\n';
}

ClientSocketPtr MasterSocketPtr::acceptNewClient() const
{
    int newClientFd = accept((*m_pointer)->m_fileDescriptor, NULL, NULL);

    if (newClientFd < 0)
        throw std::runtime_error("accept: " + std::string(std::strerror(errno)));

    log << "New client accepted and assigned to fd: " << newClientFd << '\n';

    return new ClientSocket(newClientFd, *this);
}

ServerPtr MasterSocket::serverForName(const std::string& name) const
{
    std::vector<ServerPtr>::const_iterator curr;
    for (curr = --m_servers.end(); curr != m_servers.begin(); --curr)
    {
        if (std::find((*curr)->hostNames().begin(), (*curr)->hostNames().end(), name) != (*curr)->hostNames().end())
            break;
    }
    return *curr;
}

MasterSocket::~MasterSocket()
{
    if (close(m_fileDescriptor) < 0)
        throw std::runtime_error("close: " + std::string(std::strerror(errno)));
    log << "MasterSocket with file descriptor " << m_fileDescriptor << " closed\n";
}

}