/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileWriteTask.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/29 19:15:41 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/29 20:54:29 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILEWRITETASK_HPP
# define FILEWRITETASK_HPP

#include "IO/IOTask.hpp"

#include "Socket/ClientSocket.hpp"
#include "HTTP/HTTPResponse.hpp"
#include "RequestHandler/Resource.hpp"

namespace webserv
{

class FileWriteTask : public IWriteTask
{
public:
    FileWriteTask(const std::vector<NewFileResourcePtr>& newFileResources,
                  const HTTPRequestPtr& request,
                  const HTTPResponsePtr& response,
                  const ClientSocketPtr& clientSocket);
    
    int fd() /*override*/;
    void write() /*override*/;

private:
    std::vector<NewFileResourcePtr> m_newFileResources;
    HTTPRequestPtr m_request;
    HTTPResponsePtr m_response;
    ClientSocketPtr m_clientSocket;

    std::vector<NewFileResourcePtr>::iterator m_newFileResourceCurr;
    std::vector<MultipartFormData>::iterator m_multipartDataCurr;
    uint64 m_idx;
};

}

#endif // FILEWRITETASK_HPP