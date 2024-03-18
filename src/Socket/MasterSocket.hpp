/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MasterSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/17 14:45:53 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/10 17:40:05 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MASTERSOCKET_HPP
# define MASTERSOCKET_HPP

#include <vector>
#include <string>

#include "Utils/Utils.hpp"
#include "ConfigParser/ConfigParser.hpp"

namespace webserv
{

class ClientSocket;
typedef SharedPtr<ClientSocket> ClientSocketPtr;

class MasterSocket;
typedef SharedPtr<MasterSocket> MasterSocketPtr;
class MasterSocket
{
public:
    MasterSocket(uint16 port);
    
    inline int fileDescriptor() const { return m_fileDescriptor; }

    inline void addServerConfig(const ServerConfig& config) { m_serverConfigs.push_back(config); }

    ClientSocketPtr acceptNewClient(const MasterSocketPtr& _this) const;
    const ServerConfig& configForHost(const std::string& hostname) const;

    ~MasterSocket();

private:
    const int m_fileDescriptor;
    std::vector<ServerConfig> m_serverConfigs;
};

}

#endif // MASTERSOCKET_HPP