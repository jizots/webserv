/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MasterSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/17 15:46:39 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/10 17:39:16 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket/MasterSocket.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>

#include "Socket/ClientSocket.hpp"

namespace webserv
{

MasterSocket::MasterSocket(uint16 port) : m_fileDescriptor(socket(AF_INET, SOCK_STREAM, 0))
{
    if (m_fileDescriptor < 0)
        throw std::runtime_error("socket: " + std::string(std::strerror(errno)));
    
    sockaddr_in address = (sockaddr_in){
        .sin_family = AF_INET, .sin_port = htons(port), .sin_addr=(in_addr){.s_addr=INADDR_ANY}, .sin_zero = {}
    };

    if (bind(m_fileDescriptor, reinterpret_cast<const sockaddr*>(&address), sizeof(sockaddr_in)) < 0)
        throw std::runtime_error("bind: " + std::string(std::strerror(errno)));

    if (listen(m_fileDescriptor, 10) < 0) //! is 10 OK ?
        throw std::runtime_error("listen: " + std::string(std::strerror(errno)));

    log << "Master socket created for port: " << port << '\n';
}

ClientSocketPtr MasterSocket::acceptNewClient(const MasterSocketPtr& _this) const
{
    int newClientFd = ::accept(m_fileDescriptor, NULL, NULL);

    if (newClientFd < 0)
        throw std::runtime_error("accept: " + std::string(std::strerror(errno)));

    log << "New client accepted and assigned to fd: " << newClientFd << '\n';

    return new ClientSocket(newClientFd, _this);
}

const ServerConfig& MasterSocket::configForHost(const std::string& hostname) const
{
    std::vector<ServerConfig>::const_iterator curr;
    for (curr = --m_serverConfigs.end(); curr != m_serverConfigs.begin(); --curr)
    {
        if (std::find(curr->server_names.begin(), curr->server_names.end(), hostname) != curr->server_names.end())
            break;
    }
    return *curr;
}

MasterSocket::~MasterSocket()
{
    if (::close(m_fileDescriptor) < 0)
        throw std::runtime_error("close: " + std::string(std::strerror(errno)));
    log << "MasterSocket with file descriptor " << m_fileDescriptor << " closed\n";
}

}