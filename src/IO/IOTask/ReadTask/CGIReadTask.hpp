/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIReadTask.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:13:31 by tchoquet          #+#    #+#             */
/*   Updated: 2024/05/08 13:56:20 by tchoquet         ###   ########.fr       */
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
#include "Parser/UriParser/UriPaser.hpp"

namespace webserv
{

class CGIReadTask : public IReadTask
{
public:
    CGIReadTask(const FileDescriptor& fd, int pid, const HTTPResponsePtr& response, const RequestHandlerPtr& handler);

    inline const FileDescriptor& fd() /*override*/ { return m_fd; }
    void read() /*override*/;

    inline void setRelatedWriteTaskPtr(CGIWriteTask* ptr) { m_writeTaskPtr = ptr; }

    ~CGIReadTask();

private:
    enum Status { header, body };

    template<int CODE>
    void erroCodeResponse();

    void redirectionResponse();

    int processHeaders();

    FileDescriptor m_fd;
    int m_pid;
    HTTPResponsePtr m_response;
    RequestHandlerPtr m_handler;

    std::map<std::string, std::string> m_headers;
    CGIResponseParser m_parser;

    CGIWriteTask* m_writeTaskPtr;
    Status m_status;

    void (CGIReadTask::*m_createRespFunc)();
};

}

#endif // CGIREADTASK_HPP