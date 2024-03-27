/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/17 16:57:24 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/21 12:23:50 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket/ClientSocket.hpp"

#include <unistd.h>
#include <cstring>

namespace webserv
{

HTTPResponsePtr ClientSocket::newEnqueuedResponse()
{
    HTTPResponsePtr resp = new HTTPResponse;
    m_responses.push(resp);
    return resp;
}

HTTPResponsePtr ClientSocket::nextResponse()
{
    if (m_responses.empty())
        return NULL;
    return m_responses.front();
}

ClientSocket::~ClientSocket()
{
    if (close(m_fileDescriptor) < 0)
        throw std::runtime_error("close: " + std::string(std::strerror(errno)));
    log << "Connection with client " << ipAddress() << " closed (fd: " << m_fileDescriptor << ")\n";
}

}