/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/11 18:50:57 by tchoquet          #+#    #+#             */
/*   Updated: 2024/02/28 17:03:37 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>

#include "ClientSocket.hpp"
#include "IOManager.hpp"

namespace webserv
{

void Server::handleRequest(const HTTPRequest& req, const ClientSocketPtr& clientSocket)
{
    int bodyFd;
    HTTPResponsePtr response = clientSocket->newEnqueuedResponse();
    m_internalRedirectionCount = 0;

    if (makeResponse(req, response, bodyFd)->isComplete)
    {
        if (clientSocket->nextResponse() == response)
            IOManager::shared().insertWriteTask(new ClientSocketWriteTask(clientSocket, *clientSocket->nextResponse()));
    }
    else
    {
        IOManager::shared().insertReadTask(new FileReadTask(bodyFd, clientSocket, response));
    }
}

HTTPResponsePtr& Server::makeResponse(const HTTPRequest& request, HTTPResponsePtr& response, int& fd)
{
    LocationDirective location = findBestLocation(request.requestedFile);
    struct stat fileStat;

    m_internalRedirectionCount++;
    if (m_internalRedirectionCount == 10)
    {
        log << "Too many internal redirection, 500 internal server error\n";
        return makeErrorResponse(500, response, location.error_page, fd);
    }
    if (m_internalRedirectionCount == 12)
        return makeErrorResponse(500, response, std::map<int, std::string>(), fd);

    if (request.isBadRequest)
        return makeErrorResponse(400, response, location.error_page, fd);
    
    std::string fileFullPath = (location.root.back() == '/' ? location.root.substr(0, location.root.length() - 1) : location.root) + request.requestedFile;
    log << "resolved file : " << fileFullPath << '\n';

    if (fileFullPath.back() == '/')
    {
        log << "Ending with '/' trying with index -> " << request.requestedFile + location.index << '\n';
        if (stat((fileFullPath + location.index).c_str(), &fileStat) == 0)
            return makeResponse(HTTPRequest(request, request.method, request.requestedFile + location.index), response, fd);

        log << fileFullPath << ": stat(): " << std::strerror(errno) << ". Using dir" << '\n';
        return makeErrorResponse(403, response, location.error_page, fd);
    }

    if (stat(fileFullPath.c_str(), &fileStat) != 0)
    {
        log << fileFullPath << ": stat(): " << std::strerror(errno) << '\n';
        if (errno == ENOENT)
            return makeErrorResponse(404, response, location.error_page, fd);

        return makeErrorResponse(500, response, location.error_page, fd);
    }

    if (S_ISDIR(fileStat.st_mode))
        return makeRedirResponse(301, response, request.requestedFile + '/', fd);

    if ((fd = open(fileFullPath.c_str(), O_RDONLY)) < 0)
    {
        log << fileFullPath << ": open(): " << strerror(errno) << ". 500 error\n";
        return makeErrorResponse(500, response, location.error_page, fd);
    }

    response->body.resize(fileStat.st_size);
    response->setContentType(fileFullPath);

    return response;
}

HTTPResponsePtr& Server::makeErrorResponse(uint32 code, HTTPResponsePtr& response, const std::map<int, std::string>& error_pages, int& fd)
{
    std::map<int, std::string>::const_iterator errorPage = error_pages.find(code);
    if (response->statusCode < 300 && errorPage != error_pages.end())
    {
        if (errorPage->second.front() == '/')
        {
            log << "internal redirection to " << errorPage->second << '\n';
            response->setStatusCode(code);
            return makeResponse(HTTPRequest(HTTPRequest(), "GET", errorPage->second), response, fd);
        }
        else
            return makeRedirResponse(302, response, errorPage->second, fd);
    }

    log << "built in error page\n";

    response->setStatusCode(code);
    response->contentType = HTTP::html;
    response->setBody(BUILT_IN_ERROR_PAGE(code, response->statusDescription));
    
    response->completeResponse();
    return response;
}

HTTPResponsePtr& Server::makeRedirResponse(uint32 code, HTTPResponsePtr& response, const std::string& uri, int&)
{
    log << "redirection to " << uri << '\n';

    response->setStatusCode(code);
    response->contentType = HTTP::none;
    (*response)["Location"] = uri;

    response->completeResponse();
    return response;
}

LocationDirective Server::findBestLocation(const std::string& requestedURI)
{
    std::map<uint32, LocationDirective> map;

    log << "requested URI: \"" << requestedURI << "\"\n";

    for (std::vector<LocationDirective>::const_iterator curr = m_config.locations.begin(); curr != m_config.locations.end(); ++curr) 
    {
        if (curr->location == requestedURI.substr(0, curr->location.size()))
        {
            map[curr->location.size()] = *curr;
            log << "matching location: \"" << curr->location << "\"\n";
        }
    }

    log << "using location: \"" << (--map.end())->second.location << "\"\n";
    return (--map.end())->second;
}

} // namespace webserv
