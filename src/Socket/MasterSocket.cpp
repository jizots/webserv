/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MasterSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/17 15:46:39 by tchoquet          #+#    #+#             */
/*   Updated: 2024/05/07 20:44:36 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket/MasterSocket.hpp"

#include <cstring>
#include <algorithm>

#include "Socket/ClientSocket.hpp"

namespace webserv
{

MasterSocket::MasterSocket(uint16 port) : m_fileDescriptor(webserv::socket(AF_INET, SOCK_STREAM, 0)), m_port(port)
{
    if (m_fileDescriptor)
    {
        int reuse = 1;
        if (setsockopt(m_fileDescriptor, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
            throw std::runtime_error("setsockopt(SO_REUSEADDR): " + std::string(std::strerror(errno)));

#ifdef SO_REUSEPORT
        if (setsockopt(m_fileDescriptor, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
            throw std::runtime_error("setsockopt(SO_REUSEPORT): " + std::string(std::strerror(errno)));
#endif
        const sockaddr_in address = (sockaddr_in)
        {
            .sin_family = AF_INET,
            .sin_port   = htons(port),
            .sin_addr   = (in_addr){ .s_addr = INADDR_ANY },
            .sin_zero   = {}
        };

        if (bind(m_fileDescriptor, reinterpret_cast<const sockaddr*>(&address), sizeof(sockaddr_in)) < 0)
            throw std::runtime_error("bind: " + std::string(std::strerror(errno)));

        if (listen(m_fileDescriptor, SOMAXCONN) < 0)
            throw std::runtime_error("listen: " + std::string(std::strerror(errno)));

        log << "Master socket created for port: " << port << '\n';
    }
    else
        throw std::runtime_error("socket: " + std::string(std::strerror(errno)));
}

ClientSocketPtr MasterSocket::acceptNewClient(const MasterSocketPtr& _this) const
{
    struct sockaddr address;
    socklen_t len;

    if (FileDescriptor newClientFd = accept(m_fileDescriptor, &address, &len))
    {
        ClientSocket* clientSocket = new ClientSocket(newClientFd, _this, (struct sockaddr_in&)address);
        log << "Connection from ip " << clientSocket->ipAddress() << " accepted and assigned to fd: " << newClientFd << '\n';
        return clientSocket;
    }
    return NULL;
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

}