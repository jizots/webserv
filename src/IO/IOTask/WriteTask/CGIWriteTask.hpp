/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIWriteTask.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:16:17 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/05 16:05:48 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIWRITETASK_HPP
# define CGIWRITETASK_HPP

#include "IO/IOTask/IOTask.hpp"

#include "HTTP/HTTPRequest.hpp"
#include "RequestHandler/RequestHandler.hpp"

namespace webserv
{

class CGIWriteTask : public IWriteTask
{
public:
    CGIWriteTask(const FileDescriptor& fd, const HTTPRequestPtr& request, const RequestHandlerPtr& handler);

    inline const FileDescriptor& fd() /*override*/ { return m_fd; }
    void write() /*override*/;

private:
    FileDescriptor m_fd;
    HTTPRequestPtr m_request;
    RequestHandlerPtr m_handler;
    uint64 m_idx;
};

}

#endif // CGIWRITETASK_HPP