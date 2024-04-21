/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/17 16:57:24 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/06 18:18:29 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket/ClientSocket.hpp"

namespace webserv
{

ClientSocket::ClientSocket(const FileDescriptor& fd, const MasterSocketPtr& masterSocket, const struct sockaddr_in& address)
    : m_fileDescriptor(fd), m_masterSocket(masterSocket), m_address(address)
{
}

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

}