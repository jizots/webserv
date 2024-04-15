/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocketReadTask.hpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:02:41 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/08 18:29:21 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTSOCKETREADTASK_HPP
# define CLIENTSOCKETREADTASK_HPP

#include "IO/IOTask.hpp"

#include "Socket/ClientSocket.hpp"
#include "Parser/HTTPRequestParser/HTTPRequestParser.hpp"
#include "RequestHandler/RequestHandler.hpp"

namespace webserv
{

class ClientSocketReadTask : public IReadTask
{
public:
    ClientSocketReadTask(const ClientSocketPtr& clientSocket);

    int fd() /*override*/;
    void read() /*override*/;

private:
    enum Status { requestLine, header, body };

    ClientSocketPtr m_clientSocket;

    HTTPRequestPtr m_request;
    HTTPRequestParser m_parser;
    RequestHandlerPtr m_handler;

    Status m_status;
};

}

#endif // CLIENTSOCKETREADTASK_HPP