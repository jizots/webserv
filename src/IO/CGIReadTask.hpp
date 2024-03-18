/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIReadTask.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:13:31 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/17 16:39:10 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIREADTASK_HPP
# define CGIREADTASK_HPP

#include "IO/IOTask.hpp"

#include "Socket/ClientSocket.hpp"
#include "HTTP/HTTPResponse.hpp"
#include "HTTP/CGIParser.hpp"

namespace webserv
{

class CGIReadTask : public IReadTask
{
public:
    CGIReadTask(int readFd, IWriteTask* cgiWritetask, const ClientSocketPtr& clientSocket, const HTTPResponsePtr& response);

    int fd() /*override*/;
    void read() /*override*/;

    ~CGIReadTask() /*override*/;

private:
    int m_readFd;
    IWriteTask* m_cgiWritetask;
    ClientSocketPtr m_clientSocket;
    HTTPResponsePtr m_response;
    CGIParser m_parser;
};

}

#endif // CGIREADTASK_HPP