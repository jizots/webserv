/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MasterSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/17 14:45:53 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/08 18:28:51 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MASTERSOCKET_HPP
# define MASTERSOCKET_HPP

#include <vector>
#include <string>

#include "Utils/Utils.hpp"
#include "Parser/ConfigParser/ConfigParser.hpp"

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
    inline uint16 port() { return m_port; }

    inline void addServerConfig(const ServerConfig& config) { m_serverConfigs.push_back(config); }

    ClientSocketPtr acceptNewClient(const MasterSocketPtr& _this) const;
    const ServerConfig& configForHost(const std::string& hostname) const;
    const ServerConfig& defaultConfig() const { return m_serverConfigs.front(); };

    ~MasterSocket();

private:
    const int m_fileDescriptor;
    const uint16 m_port;
    std::vector<ServerConfig> m_serverConfigs;
};

}

#endif // MASTERSOCKET_HPP