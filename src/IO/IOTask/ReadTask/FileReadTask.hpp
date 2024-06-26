/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileReadTask.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:06:31 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/21 14:21:28 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILEREADTASK_HPP
# define FILEREADTASK_HPP

#include "IO/IOTask/IOTask.hpp"

#include "RequestHandler/Resource/StaticFileResource.hpp"
#include "HTTP/HTTPResponse.hpp"
#include "RequestHandler/RequestHandler.hpp"

namespace webserv
{

class FileReadTask : public IReadTask
{
public:
    FileReadTask(const StaticFileResourcePtr& resource, const HTTPResponsePtr& response, const RequestHandlerPtr& handler);

    inline const FileDescriptor& fd() /*override*/ { return m_resource->fileDescriptor(); }
    void read() /*override*/;

private:
    StaticFileResourcePtr m_resource;
    HTTPResponsePtr m_response;
    RequestHandlerPtr m_handler;
    uint64 m_idx;
};

}

#endif // FILEREADTASK_HPP