/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/17 14:56:12 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/06 17:10:38 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTSOCKET_HPP
# define CLIENTSOCKET_HPP

#include "Socket/MasterSocket.hpp"
#include "HTTP/HTTPResponse.hpp"

#include <queue>

namespace webserv
{

class ClientSocket
{
friend ClientSocketPtr MasterSocket::acceptNewClient(const MasterSocketPtr&) const;

public:
    inline int fileDescriptor() { return m_fileDescriptor; }
    inline const MasterSocketPtr& masterSocket() { return m_masterSocket; }

    HTTPResponsePtr newEnqueuedResponse();
    HTTPResponsePtr nextResponse();
    inline void popResponse() { m_responses.pop(); }

    ~ClientSocket();

private:
    inline ClientSocket(int fd, const MasterSocketPtr& masterSocket) : m_fileDescriptor(fd), m_masterSocket(masterSocket) {}

    const int m_fileDescriptor;
    const MasterSocketPtr m_masterSocket;
    std::queue<HTTPResponsePtr> m_responses;
};
typedef SharedPtr<ClientSocket> ClientSocketPtr;

}

#endif // CLIENTSOCKET_HPP