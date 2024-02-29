/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/11 18:50:18 by tchoquet          #+#    #+#             */
/*   Updated: 2024/02/28 15:20:55 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include <map>

#include "Utils/Utils.hpp"
#include "ConfigParser/ConfigParser.hpp"
#include "HTTP/HTTP.hpp"

namespace webserv
{

class ClientSocket;
typedef SharedPtr<ClientSocket> ClientSocketPtr;

class Server
{
public:
    inline Server(const ServerConfig& conf) : m_config(conf) {}

    inline const std::vector<std::string>& hostNames() const { return m_config.server_names; }
    void handleRequest(const HTTPRequest& req, const ClientSocketPtr& clientSocket);

private:
    HTTPResponsePtr& makeResponse(const HTTPRequest& request, HTTPResponsePtr& response, int& fd);
    HTTPResponsePtr& makeErrorResponse(uint32 code, HTTPResponsePtr& response, const std::map<int, std::string>& error_pages, int& fd);
    HTTPResponsePtr& makeRedirResponse(uint32 code, HTTPResponsePtr& response, const std::string& uri, int& fd);

    LocationDirective findBestLocation(const std::string& requestedURI);

    const ServerConfig m_config;
    uint32 m_internalRedirectionCount;
};
typedef SharedPtr<Server> ServerPtr;

}

#endif // SERVER_HPP