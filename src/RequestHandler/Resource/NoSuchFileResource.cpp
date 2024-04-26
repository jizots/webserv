/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NoSuchFileResource.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/21 16:58:06 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/26 12:58:38 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler/Resource/NoSuchFileResource.hpp"

#include "Utils/Logger.hpp"

namespace webserv
{

NoSuchFileResource::NoSuchFileResource(const std::string& path) : Resource(path)
{

}

int NoSuchFileResource::open()
{
    m_fd = webserv::open(m_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (!m_fd)
    {
        log << "\"" << m_path << "\": open(): " << std::strerror(errno) << '\n';
        return -1;
    }
    log << "File \"" << m_path << "\" opened for writing (fd: " << m_fd << ")\n"; 
    return 0;
}

}
