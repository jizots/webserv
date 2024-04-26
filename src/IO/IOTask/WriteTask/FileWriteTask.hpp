/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileWriteTask.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/21 17:32:25 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/21 17:51:38 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILEWRITETASK_HPP
# define FILEWRITETASK_HPP

#include "IO/IOTask/IOTask.hpp"

#include "HTTP/HTTPRequest.hpp"
#include "RequestHandler/RequestHandler.hpp"
#include "RequestHandler/Resource/NoSuchFileResource.hpp"

namespace webserv
{

class FileWriteTask : public IWriteTask
{
public:
    FileWriteTask(const NoSuchFileResourcePtr& resource, const HTTPRequestPtr& request, const RequestHandlerPtr& handler);
    
    inline const FileDescriptor& fd() /*override*/ { return m_resource->fileDescriptor(); }
    void write() /*override*/;

private:
    NoSuchFileResourcePtr m_resource;
    HTTPRequestPtr m_request;
    RequestHandlerPtr m_handler;
    uint64 m_idx;
};

}

#endif // FILEWRITETASK_HPP