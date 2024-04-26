/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Resource.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/13 13:26:07 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/26 12:57:27 by tchoquet         ###   ########.fr       */
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
#include "RequestHandler/Resource/StaticFileResource.hpp"
#include "RequestHandler/Resource/DirectoryResource.hpp"
#include "RequestHandler/Resource/NoSuchFileResource.hpp"

namespace webserv
{

SharedPtr<Resource> Resource::create(const std::string& uri, const LocationDirective& location)
{
    if (location.accepted_cgi_extension.empty() == false)
    {
        std::string scriptURI = uri;
        while (location.isMatching(scriptURI))
        {
            struct stat st;
            const std::string scriptPath = location.translateURI(scriptURI);
            
            if (::stat(scriptPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode) == false)
            {
                const std::string::size_type dotPos = scriptPath.find_last_of(".");
                const std::string extension = dotPos != std::string::npos ? scriptPath.substr(dotPos) : "";

                std::map<std::string, std::string>::const_iterator it = location.accepted_cgi_extension.find(extension);
                if (it == location.accepted_cgi_extension.end())
                    it = location.accepted_cgi_extension.find(".*");
                
                if (it != location.accepted_cgi_extension.end())
                {
                    CGIResourcePtr cgiResource = new CGIResource(scriptPath, it->second);

                    cgiResource->setEnvp("SCRIPT_NAME", scriptURI);
                    cgiResource->setEnvp("SCRIPT_FILENAME",  scriptPath); // * for PHP

                    if (scriptURI.size() < uri.size())
                    {
                        cgiResource->setEnvp("PATH_INFO", uri.substr(scriptURI.size()));
                        cgiResource->setEnvp("PATH_TRANSLATED", location.translateURI(uri.substr(scriptURI.size())));
                    }

                    if (it->second.size() > 0)
                        cgiResource->setEnvp("REDIRECT_STATUS", "200"); // * for PHP

                    return cgiResource;
                }
            }

            if (scriptURI.find_last_of("/") == std::string::npos)
                break;
            scriptURI = scriptURI.substr(0, scriptURI.find_last_of("/"));
        }
    }

    const std::string path = location.translateURI(uri);
    if (::access(path.c_str(), F_OK) != 0)
    {
        log << "\"" << location.translateURI(uri) << "\": access(): " << std::strerror(errno) << '\n';
        return new NoSuchFileResource(location.translateURI(uri));
    }

    struct stat stat;

    if (::stat(path.c_str(), &stat) < 0)
    {
        log << "\"" << path << "\": stat(): " << std::strerror(errno) << '\n';
        return NULL;
    }

    if (S_ISDIR(stat.st_mode))
    {
        log << "S_ISDIR(stat.st_mode): true\n";
        return new DirectoryResource(path);
    }

    return new StaticFileResource(path, stat);
}

Resource::Resource(const std::string& path)
    : m_path(path)
{
}

}
