/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIReadTask.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:13:31 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/05 18:41:45 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIREADTASK_HPP
# define CGIREADTASK_HPP

#include "IO/IOTask/IOTask.hpp"

#include <map>
#include <vector>

#include "HTTP/HTTPResponse.hpp"
#include "RequestHandler/RequestHandler.hpp"
#include "Parser/CGIResponseParser/CGIResponseParser.hpp"
#include "IO/IOTask/WriteTask/CGIWriteTask.hpp"

namespace webserv
{

class CGIReadTask : public IReadTask
{
public:
    CGIReadTask(const FileDescriptor& fd, const HTTPResponsePtr& response, const RequestHandlerPtr& handler);

    inline const FileDescriptor& fd() /*override*/ { return m_fd; }
    void read() /*override*/;

    inline void setRelatedWriteTaskPtr(CGIWriteTask* ptr) { m_writeTaskPtr = ptr; }

private:
    enum Status { header, body };

    FileDescriptor m_fd;
    HTTPResponsePtr m_response;
    RequestHandlerPtr m_handler;

    CGIResponseParser m_parser;
    std::map<std::string, std::string> m_headers;

    CGIWriteTask* m_writeTaskPtr;
    Status m_status;
};

}

#endif // CGIREADTASK_HPP