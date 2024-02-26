/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MasterSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/17 14:45:53 by tchoquet          #+#    #+#             */
/*   Updated: 2024/02/21 19:40:43 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MASTERSOCKET_HPP
# define MASTERSOCKET_HPP

#include <vector>
#include <string>

#include "Utils/Utils.hpp"
#include "Server.hpp"

namespace webserv
{

class ClientSocket;
typedef SharedPtr<ClientSocket> ClientSocketPtr;

class MasterSocket;
class MasterSocketPtr : public SharedPtr<MasterSocket>
{
public:
    inline MasterSocketPtr()                                            : SharedPtr<MasterSocket>()    {}
    inline MasterSocketPtr(const MasterSocketPtr& cp)                   : SharedPtr<MasterSocket>(cp)  {}
    template<typename Y> inline MasterSocketPtr(const SharedPtr<Y>& cp) : SharedPtr<MasterSocket>(cp)  {}
    inline MasterSocketPtr(MasterSocket* ptr)                           : SharedPtr<MasterSocket>(ptr) {}

    ClientSocketPtr acceptNewClient() const;
};

class MasterSocket
{
public:
    MasterSocket(uint32 port);
    
    inline int fileDescriptor() const { return m_fileDescriptor; }

    inline void addServer(const ServerPtr& serv) { m_servers.push_back(serv); }

    friend ClientSocketPtr MasterSocketPtr::acceptNewClient() const;
    ServerPtr serverForName(const std::string& name) const;

    ~MasterSocket();

private:
    const int m_fileDescriptor;
    std::vector<ServerPtr> m_servers;
};

}

#endif // MASTERSOCKET_HPP