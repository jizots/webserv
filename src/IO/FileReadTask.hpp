/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileReadTask.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:06:31 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/20 18:40:47 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILEREADTASK_HPP
# define FILEREADTASK_HPP

#include "IO/IOTask.hpp"

#include "Socket/ClientSocket.hpp"
#include "HTTP/HTTPResponse.hpp"
#include "RequestHandler/Resource.hpp"

namespace webserv
{

class FileReadTask : public IReadTask
{
public:
    FileReadTask(DiskResourcePtr resource, const ClientSocketPtr& clientSocket, const HTTPResponsePtr& response);

    int fd() /*override*/;
    void read() /*override*/;

private:
    DiskResourcePtr m_resource;
    ClientSocketPtr m_clientSocket;
    HTTPResponsePtr m_response;
};

}

#endif // FILEREADTASK_HPP