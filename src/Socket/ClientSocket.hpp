/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/17 14:56:12 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/06 18:20:22 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTSOCKET_HPP
# define CLIENTSOCKET_HPP

#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <queue>

#include "Socket/MasterSocket.hpp"
#include "HTTP/HTTPResponse.hpp"

namespace webserv
{

class ClientSocket
{
friend ClientSocketPtr MasterSocket::acceptNewClient(const MasterSocketPtr&) const;

public:
    inline const FileDescriptor& fileDescriptor() { return m_fileDescriptor; }
    inline const MasterSocketPtr& masterSocket() { return m_masterSocket; }
    inline std::string ipAddress() { return inet_ntoa(m_address.sin_addr); }

    HTTPResponsePtr newEnqueuedResponse();
    HTTPResponsePtr nextResponse();
    inline void popResponse() { m_responses.pop(); }

private:
    ClientSocket(const FileDescriptor& fd, const MasterSocketPtr& masterSocket, const struct sockaddr_in& address);

    const FileDescriptor m_fileDescriptor;
    const MasterSocketPtr m_masterSocket;
    const struct sockaddr_in m_address;
    std::queue<HTTPResponsePtr> m_responses;
};
typedef SharedPtr<ClientSocket> ClientSocketPtr;

}

#endif // CLIENTSOCKET_HPP