/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Resource.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/13 13:26:07 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/02 12:56:13 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler/Resource/Resource.hpp"

#include <string>
#include <map>
#include <sys/stat.h>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/unistd.h>

#include "Utils/Macros.hpp"
#include "Utils/Logger.hpp"
#include "RequestHandler/Resource/CGIResource.hpp"
#include "RequestHandler/Resource/ReadFileResource.hpp"
#include "RequestHandler/Resource/DirectoryResource.hpp"

namespace webserv
{

SharedPtr<Resource> Resource::create(const std::string& path, const std::map<std::string, std::string>& cgiExt)
{
    const std::string::size_type firstDot = path.find_first_of('.');
    const std::string::size_type nextSlash = firstDot == std::string::npos ? std::string::npos : path.find_first_of('/', firstDot);
    const std::string extention = firstDot == std::string::npos ? "" : path.substr(firstDot, nextSlash == std::string::npos ? std::string::npos : nextSlash - firstDot);

    for (std::map<std::string, std::string>::const_iterator curr = cgiExt.begin(); curr != cgiExt.end(); ++curr)
    {
        if (curr->first == extention)
        {
            if (::access((curr->second.size() > 0 ? curr->second : path.substr(0, nextSlash)).c_str(), F_OK) != 0)
            {
                log << (curr->second.size() > 0 ? curr->second : path.substr(0, nextSlash)) << ": no such file or directory" << '\n';
                return NULL;
            }
            
            if (::access((curr->second.size() > 0 ? curr->second : path.substr(0, nextSlash)).c_str(), X_OK) != 0)
            {
                log << (curr->second.size() > 0 ? curr->second : path.substr(0, nextSlash)) << ": not executable" <<  '\n';
                return NULL;
            }

            return new CGIResource(curr->second.size() > 0 ? curr->second : path.substr(0, nextSlash),
                                   curr->second.size() > 0 ? path.substr(0, nextSlash) : "");
        }
    }

    struct stat stat;

    if (::stat(path.c_str(), &stat) < 0)
    {
        log << path << ": stat(): " << std::strerror(errno) << '\n';
        return NULL;
    }

    if ((stat.st_mode & S_IRUSR) == 0)
    {
        log << path << ": No read access\n";
        return NULL;
    }

    if (S_ISDIR(stat.st_mode))
        return new DirectoryResource(path);

    return new ReadFileResource(path, stat);
}

Resource::Resource(const std::string& path)
    : m_path(path)
{
}

}
