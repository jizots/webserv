/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 18:32:54 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/25 19:11:11 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler/RequestHandler.hpp"

#include <algorithm>

#include "IO/IOManager.hpp"
#include "IO/ClientSocketWriteTask.hpp"
#include "IO/CGIWriteTask.hpp"
#include "IO/CGIReadTask.hpp"
#include "IO/FileReadTask.hpp"

namespace webserv
{

RequestHandler::RequestHandler(const HTTPRequestPtr& request, const ClientSocketPtr& clientSocket)
    : m_request(request), m_clientSocket(clientSocket),
      m_config(clientSocket->masterSocket()->defaultConfig()), m_location(m_config.locations.back()),
      m_response(clientSocket->newEnqueuedResponse()),
      m_shouldEndConnection(false), m_internalRedirectionCount(0)
{
}

int RequestHandler::processRequestLine()
{
    log << "processing requestLine\n";

    m_shouldEndConnection = true;

    if (m_request->isBadRequest)
        return 400;

    if (m_request->httpVersionMajor != 1 || m_request->httpVersionMinor != 1)
        return 505;

    m_shouldEndConnection = false;
    return 0;
}

int RequestHandler::processHeaders()
{
    log << "processing headers\n";

    if (m_request->isBadRequest)
    {
        m_shouldEndConnection = true;
        return 400;
    }

    m_config = m_clientSocket->masterSocket()->configForHost(m_request->host);
    m_location = m_config.bestLocation(m_request->uri);
    
    std::map<std::string, std::string>::const_iterator it = m_request->headers.find("connection");
    if (it != m_request->headers.end())
    {
        if (it->second == "close")
            m_response->headers["connection"] = "close";

        else if (it->second == "keep-alive")
        {
            m_response->headers["connection"] = "keep-alive";
            m_shouldEndConnection = false;
        }

        else
            return 400;
    }
    
    return 0;
}

void RequestHandler::makeErrorResponse(int code)
{
    log << "error code " << code << '\n';

    m_response->headers.erase("location");

    if (m_response->statusCode == 200)
    {
        std::map<int, std::string>::const_iterator userPage = m_location.error_page.find(code);

        if (userPage != m_location.error_page.end() && *userPage->second.begin() != '/')
            return makeRedirectionResponse(302, userPage->second);

        m_response->setStatusCode(code);

        if (userPage != m_location.error_page.end() && *userPage->second.begin() == '/')
            return internalRedirection("GET", std::string(userPage->second), "");
    }

    m_response->makeBuiltInResponse(code);
}

void RequestHandler::makeRedirectionResponse(int code, const std::string& location)
{
    log << "redirection code " << code << " to \"" << location << "\"\n";

    m_response->headers["location"] = location;

    if (m_response->statusCode == 200)
    {
        std::map<int, std::string>::const_iterator userPage = m_location.error_page.find(code);

        m_response->setStatusCode(code);

        if (userPage != m_location.error_page.end() && *userPage->second.begin() != '/')
            return makeRedirectionResponse(302, userPage->second);

        if (userPage != m_location.error_page.end() && *userPage->second.begin() == '/')
            return internalRedirection("GET", std::string(userPage->second), "");
    }

    m_response->makeBuiltInResponse(code);
}

void RequestHandler::runTasks(const RequestHandlerPtr& _this)
{
    log << "running task\n";

    if (m_response->isComplete)
    {    
        if (m_clientSocket->nextResponse() == m_response)
            IOManager::shared().insertWriteTask(new ClientSocketWriteTask(m_clientSocket, m_clientSocket->nextResponse()));
        return;
    }

    if(m_responseResource->open() != 0)
    {
        if (errno == ENOENT)
            return makeErrorResponse(404);
        return makeErrorResponse(500);

        return runTasks(_this);
    }

    if (DiskResourcePtr resource = m_responseResource.dynamicCast<DiskResource>())
    {
        if (resource->readFd() > 0)
            IOManager::shared().insertReadTask(new FileReadTask(resource, m_clientSocket, m_response));
    }

    else if (CGIProgramPtr resource = m_responseResource.dynamicCast<CGIProgram>())
    {
        IWriteTask* cgiWriteTask = new CGIWriteTask(resource, m_request);

        IOManager::shared().insertWriteTask(cgiWriteTask);
        IOManager::shared().insertReadTask(new CGIReadTask(resource, cgiWriteTask, m_clientSocket, m_response, _this));
    }
}

void RequestHandler::internalRedirection(const std::string& method, const std::string& uri, const std::string& query)
{
    if (m_internalRedirectionCount > 0)
        log << "internal redirection uri \"" << uri << "\"\n";
    if (m_internalRedirectionCount == 10)
    {
        log << "too many internal redirection\n";
        m_internalRedirectionCount++;
        return makeErrorResponse(500);
    }
    if (m_internalRedirectionCount >= 12)
    {
        log << "too many internal redirection\n";
        m_internalRedirectionCount++;
        return makeErrorResponse(500);
    }
    m_internalRedirectionCount++;
    
    m_location = m_config.bestLocation(uri);

    if (std::find(m_location.accepted_methods.begin(), m_location.accepted_methods.end(), method) == m_location.accepted_methods.end())
        return makeErrorResponse(405);

    if (*(--uri.end()) == '/')
    {
        log << "Ending with '/', ";
        if (*m_location.index.begin() == '/')
            return internalRedirection(method, m_location.index, "");

        log << "checking index\n";
        if ((m_responseResource = Resource::create(uri + m_location.index, m_location.root, m_location.accepted_cgi_extension))) 
            return internalRedirection(method, uri + m_location.index, "");

        log << "index not usable, using dir\n";
        if (m_location.autoindex)
            return makeResponseAutoindex(uri);

        log << "auto index not allowed\n";
        return makeErrorResponse(403);
    }

    if (!(m_responseResource = Resource::create(uri, m_location.root, m_location.accepted_cgi_extension)))
    {
        if (errno == ENOENT || errno == ENOTDIR)
            return makeErrorResponse(404);
        return makeErrorResponse(500);
    }

    if (DiskResourcePtr resource = m_responseResource.dynamicCast<DiskResource>())
    {
        if (resource->isDIR())
            return makeRedirectionResponse(301, uri + '/');

        if (method == "POST") // ! Must be uppercase
            return makeErrorResponse(405);

        m_response->headers["Content-Type"] = resource->contentType();
        m_response->headers["Content-Length"] = to_string(resource->contentLength());
        m_response->body.resize(resource->contentLength());
    }

    else if (CGIProgramPtr cgiProg = m_responseResource.dynamicCast<CGIProgram>())
    {
        cgiProg->completeEnvp(*m_request, method, query, m_clientSocket, m_config);
    }
}

}
