/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 18:32:54 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/18 11:38:32 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler/RequestHandler.hpp"

#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sstream>
#include <algorithm>

#include "IO/IOManager.hpp"
#include "IO/ClientSocketWriteTask.hpp"
#include "IO/CGIWriteTask.hpp"
#include "IO/CGIReadTask.hpp"
#include "IO/FileReadTask.hpp"

namespace webserv
{

static LocationDirective findBestLocation(const ServerConfig& config, const std::string& uri)
{
    std::map<uint32, LocationDirective> map;

    log << "requested URI: \"" << uri << "\"\n";

    for (std::vector<LocationDirective>::const_iterator curr = config.locations.begin(); curr != config.locations.end(); ++curr) 
    {
        if (curr->location == uri.substr(0, curr->location.size()))
        {
            map[curr->location.size()] = *curr;
            log << "matching location: \"" << curr->location << "\"\n";
        }
    }

    log << "using location: \"" << (--map.end())->second.location << "\"\n";
    return (--map.end())->second;
}

RequestHandler::RequestHandler(const ClientSocketPtr& clientSocket)
    : m_clientSocket(clientSocket), m_response(clientSocket->newEnqueuedResponse()),
      m_needBody(false), m_shouldEndConnection(false)
{
}

void RequestHandler::processRequestLine(const HTTPRequest& request)
{
    m_internalRedirectionCount = 0;
    if (request.isBadRequest)
        makeResponseCode(400, std::map<int, std::string>());
}

void RequestHandler::processHeaders(const HTTPRequest& request)
{
    m_internalRedirectionCount = 0;
    m_config = m_clientSocket->masterSocket()->configForHost(request.host);

    if (request.isBadRequest)
    {
        m_shouldEndConnection = true;
        makeResponseCode(400, findBestLocation(m_config, request.uri).error_page);
    }
    else
    {
        std::map<std::string, std::string>::const_iterator it = request.headers.find("connection");
        if (it != request.headers.end() && it->second.size() > 0)
        {
            if (it->second == "close")
                m_shouldEndConnection = true;

            else if (it->second == "keep-alive")
                m_shouldEndConnection = false;

            else
                return processHeaders(HTTPRequest(request, true));
        }
            
        makeResponse(request);
    }

    if (m_requestedResource.isCGI() && request.contentLength > 0)
        return (void)(m_needBody = true);
}

void RequestHandler::handleRequest(const HTTPRequest& request)
{
    if (m_response->isComplete)
    {
        IOManager::shared().insertWriteTask(new ClientSocketWriteTask(m_clientSocket, m_response));
        return;
    }

    IWriteTask* cgiWriteTask = NULL;

    int cgiWriteFd = m_requestedResource.cgiWriteFd();
    if (cgiWriteFd > 0)
    {
        cgiWriteTask = new CGIWriteTask(cgiWriteFd, request);
        IOManager::shared().insertWriteTask(cgiWriteTask);
    }

    int cgiReadFd = m_requestedResource.cgiReadFd();
    if (cgiReadFd > 0)
    {
        IOManager::shared().insertReadTask(new CGIReadTask(cgiReadFd, cgiWriteTask, m_clientSocket, m_response));
    }

    int fileReadFd = m_requestedResource.fileReadFd();
    if (fileReadFd > 0)
        IOManager::shared().insertReadTask(new FileReadTask(fileReadFd, m_clientSocket, m_response));

    // int fileWriteFd = m_requestedResource.fileWriteFd();
    // if (fileWriteFd > 0)
        // TODO
        // ;
}

void RequestHandler::makeResponse(const HTTPRequest& request)
{
    LocationDirective location = findBestLocation(m_config, request.uri);

    if (m_internalRedirectionCount == 10)
    {
        log << "Too may internal redirection\n";
        return makeResponseCode(500, location.error_page);
    }
    if (m_internalRedirectionCount == 12)
        return makeResponseCode(500, std::map<int, std::string>());

    if (std::find(location.accepted_methods.begin(), location.accepted_methods.end(), request.method) == location.accepted_methods.end())
        return makeResponseCode(405, location.error_page);

    if (*(--request.uri.end()) == '/')
    {
        log << "Ending with '/', ";
        if (*location.index.begin() == '/')
        {
            log << "internal redirection to \"" << location.index << "\"\n";
            m_internalRedirectionCount += 1;
            return makeResponse(HTTPRequest(request, request.method, location.index));
        }

        log << "checking index\n";
        if ((m_requestedResource = Resource(RMV_LAST_SLASH(location.root) + request.uri + location.index))) 
        {
            log << "internal redirection to \"" << request.uri + location.index << "\"\n";
            m_internalRedirectionCount += 1;
            return makeResponse(HTTPRequest(request, request.method, request.uri + location.index));
        }

        log << "using dir\n";
        if (location.autoindex)
        {
            //TODO auto index
        }

        log << "auto index not allowed\n";
        return makeResponseCode(403, location.error_page);
    }

    std::string fileFullPath = RMV_LAST_SLASH(location.root) + request.uri;
    log << "resolved file: \"" << fileFullPath << "\"\n";

    if (m_requestedResource.path() != fileFullPath)
        m_requestedResource = Resource(RMV_LAST_SLASH(location.root) + request.uri);

    if (m_requestedResource == false)
    {
        if (errno == ENOENT || errno == ENOTDIR)
            return makeResponseCode(404, location.error_page);
        return makeResponseCode(500, location.error_page);
    }

    if (m_requestedResource.isDIR())
    {
        m_response->headers["Location"] = request.uri + '/';
        return makeResponseCode(301, location.error_page);
    }
    
    m_requestedResource.computeIsCGI(location.accepted_cgi_extension);
    if (m_requestedResource.isCGI())
    {
        std::vector<std::string> envp;
        makeEnvp(request, envp);
        if (m_requestedResource.createCGIProcess(envp) != 0)
        {
            if (errno == ENOENT)
                return makeResponseCode(404, location.error_page);
            return makeResponseCode(500, location.error_page);
        }
    }
    else
    {
        if (request.method == "POST") // ! Must be uppercase
            return makeResponseCode(405, location.error_page);

        if (m_requestedResource.openReadingFile() != 0)
        {
            if (errno == ENOENT)
                return makeResponseCode(404, location.error_page);
            return makeResponseCode(500, location.error_page);
        }

        m_response->headers["Content-Type"] = m_requestedResource.contentType();
        m_response->headers["Content-Length"] = to_string(m_requestedResource.contentLength());
        m_response->body.resize(m_requestedResource.contentLength());
    }
}

void RequestHandler::makeResponseCode(int code, const std::map<int, std::string>& error_page)
{
    log << "response code " << code << '\n';

    if (m_response->statusCode < 300)
    {
        std::map<int, std::string>::const_iterator userPage = error_page.find(code);
        if (userPage != error_page.end() && *userPage->second.begin() == '/')
        {
            log << "internal redirection to \"" << userPage->second << "\"\n";
            m_internalRedirectionCount += 1;
            m_response->setStatusCode(code);
            return makeResponse(HTTPRequest(HTTPRequest(), "GET", userPage->second));
        }
        else if (userPage != error_page.end() && *userPage->second.begin() != '/')
        {
            log << "redirection to \"" << userPage->second << "\"\n";
            m_response->headers["Location"] = userPage->second;
            return makeResponseCode(302, error_page);
        }
    }

    log << "built in page\n";

    m_response->setStatusCode(code);
    m_response->makeBuiltInBody();
    m_response->headers["Content-Type"] = "text/html";
    m_response->headers["Content-Length"] = to_string(m_response->body.size());
    m_response->isComplete = true;
}

void RequestHandler::makeEnvp(const HTTPRequest& request, std::vector<std::string>& envp)
{
    // TODO envp.push_back("AUTH_TYPE=");
    
    if (request.contentLength > 0)
        envp.push_back("CONTENT_LENGTH=" + to_string(request.contentLength));

    std::map<std::string, std::string>::const_iterator it = request.headers.find("content-type");
    if (it != request.headers.end())
        envp.push_back("CONTENT_TYPE=" + it->second);

    envp.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envp.push_back("PATH_INFO=" + request.httpVersionStr());
    // TODO envp.push_back("PATH_TRANSLATED=" + m_requestedResource.path());
    // TODO envp.push_back("QUERY_STRING=");
    // TODO envp.push_back("REMOTE_ADDR=");
    // ? envp.push_back("REMOTE_HOST=");
    // ? envp.push_back("REMOTE_IDENT=");
    // TODO envp.push_back("REMOTE_USER=");
    envp.push_back("REQUEST_METHOD=" + request.method);
    // TODO envp.push_back("SCRIPT_NAME=" + m_requestedResource.path());
    // TODO envp.push_back("SERVER_NAME=");
    // TODO envp.push_back("SERVER_PORT=");
    envp.push_back("SERVER_PROTOCOL=" + request.httpVersionStr());
    // TODO envp.push_back("SERVER_SOFTWARE=");
}

}