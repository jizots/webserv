/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticFileResource.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/31 18:31:23 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/26 12:58:15 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler/Resource/StaticFileResource.hpp"

#include <string>
#include <sys/stat.h>
#include <sys/fcntl.h>

#include "Utils/Macros.hpp"
#include "Utils/Logger.hpp"

namespace webserv
{

StaticFileResource::StaticFileResource(const std::string& path, const struct stat& stat)
    : Resource(path), m_stat(stat), m_contentType(path)
{
}

int StaticFileResource::open()
{
    m_fd = webserv::open(m_path.c_str(), O_RDONLY);
    if (!m_fd)
    {
        log << "\"" << m_path << "\": open(): " << std::strerror(errno) << '\n';
        return -1;
    }
    log << "File \"" << m_path << "\" opened for reading (fd: " << m_fd << ")\n"; 
    return 0;
}

}