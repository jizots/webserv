/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIReadTask.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:13:31 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/08 18:38:57 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIREADTASK_HPP
# define CGIREADTASK_HPP

#include "IO/IOTask.hpp"

#include "Socket/ClientSocket.hpp"
#include "HTTP/HTTPResponse.hpp"
#include "Parser/CGIResponseParser/CGIResponseParser.hpp"
#include "Parser/ConfigParser/ConfigParser.hpp"
#include "RequestHandler/Resource.hpp"
#include "RequestHandler/RequestHandler.hpp"

namespace webserv
{

class CGIReadTask : public IReadTask
{
public:
    CGIReadTask(CGIProgramPtr cgiProgram, IWriteTask* cgiWriteTask, const ClientSocketPtr& clientSocket, const HTTPResponsePtr& response, const RequestHandlerPtr& redirectionHandler);

    int fd() /*override*/;
    void read() /*override*/;

    ~CGIReadTask() /*override*/;

private:
    enum Status { header, body };

    CGIProgramPtr m_cgiProgram;
    IWriteTask* m_cgiWriteTask;
    ClientSocketPtr m_clientSocket;
    HTTPResponsePtr m_response;
    RequestHandlerPtr m_redirectionHandler;

    CGIResponseParser m_parser;
    std::map<std::string, std::string> m_headers;

    Status m_status;
};

}

#endif // CGIREADTASK_HPP