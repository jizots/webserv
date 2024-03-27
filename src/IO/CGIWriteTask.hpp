/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIWriteTask.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/06 16:16:17 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/23 00:49:40 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIWRITETASK_HPP
# define CGIWRITETASK_HPP

#include "IO/IOTask.hpp"

#include "HTTP/HTTPRequest.hpp"
#include "ConfigParser/ConfigParser.hpp"
#include "RequestHandler/Resource.hpp"

namespace webserv
{

class CGIWriteTask : public IWriteTask
{
public:
    CGIWriteTask(CGIProgramPtr cgiProgram, const HTTPRequestPtr& request);

    int fd() /*override*/;
    void write() /*override*/;

    ~CGIWriteTask() /*override*/;

private:
    CGIProgramPtr m_cgiProgram;
    HTTPRequestPtr m_request;
    uint64 m_idx;
};

}

#endif // CGIWRITETASK_HPP