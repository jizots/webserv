/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hotph <hotph@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 18:32:54 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/08 13:34:35 by hotph            ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "RequestHandler/RequestHandler.hpp"

#include <algorithm>

#include "IO/IOManager.hpp"
#include "IO/ClientSocketWriteTask.hpp"
#include "IO/CGIWriteTask.hpp"
#include "IO/CGIReadTask.hpp"
#include "IO/FileReadTask.hpp"
#include "IO/FileWriteTask.hpp"
#include "RequestHandler/RequestHandler.hpp"

namespace webserv
{

RequestHandler::RequestHandler(const HTTPRequestPtr& request, const ClientSocketPtr& clientSocket)
    : m_request(request), m_clientSocket(clientSocket),
      m_config(clientSocket->masterSocket()->defaultConfig()), m_location(m_config.locations.back()),
      m_response(clientSocket->newEnqueuedResponse()),
      m_needBody(false), m_shouldEndConnection(false), m_internalRedirectionCount(0)
{
}

int RequestHandler::processRequestLine()
{
    log << "processing requestLine\n";

    if (m_request->httpVersionMajor != 1 || m_request->httpVersionMinor != 1)
        return m_shouldEndConnection = true, 505;

    return 0;
}

int RequestHandler::processHeaders()
{
    log << "processing headers\n";

    std::map<std::string, std::string>::const_iterator hostIt = m_request->headers.find("host");
    if (hostIt == m_request->headers.end())
        return 400;
    if (int error = parseHeaderValue(hostIt->second, &RequestHandler::parseHost))
        return error;

    m_config = m_clientSocket->masterSocket()->configForHost(m_request->host.hostname);
    m_location = m_config.bestLocation(m_request->uri);
    
    std::map<std::string, std::string>::const_iterator connectionIt = m_request->headers.find("connection");
    if (connectionIt != m_request->headers.end())
    {
        parseHeaderValue(connectionIt->second, &RequestHandler::parseConnection);
        if (int error = parseHeaderValue(connectionIt->second, &RequestHandler::parseConnection))
            return error;
        std::vector<HTTPFieldValue>::const_iterator it = std::find_if(m_request->m_HTTPFieldValues.begin(), m_request->m_HTTPFieldValues.end(), FindValName("close"));
        if (it != m_request->m_HTTPFieldValues.end())
        {
            m_response->headers["connection"] = "close";
            m_shouldEndConnection = true;
        }
        else
        {
            it = std::find_if(m_request->m_HTTPFieldValues.begin(), m_request->m_HTTPFieldValues.end(), FindValName("keep-alive"));
            if (it != m_request->m_HTTPFieldValues.end())
            {
                m_response->headers["connection"] = "keep-alive";
                m_shouldEndConnection = false;
            }
            else
                return 400;
        }
    }

    std::map<std::string, std::string>::const_iterator contentLength = m_request->headers.find("content-length");
    if (contentLength != m_request->headers.end())
    {
        if (int error = parseHeaderValue(contentLength->second, &RequestHandler::parseContentLength))
            return error;
    }

    std::map<std::string, std::string>::const_iterator transferEncoding = m_request->headers.find("transfer-encoding");
    if (transferEncoding != m_request->headers.end())
    {
        if (int error = parseHeaderValue(transferEncoding->second, &RequestHandler::parseTransferEncoding))
            return error;
        std::vector<HTTPFieldValue>::const_iterator it = std::find_if(m_request->m_HTTPFieldValues.begin(), m_request->m_HTTPFieldValues.end(), FindValName("chunked"));
        if (it != m_request->m_HTTPFieldValues.end())
            m_request->isChunk = true;
    }

    std::map<std::string, std::string>::const_iterator contentType = m_request->headers.find("content-type");
    if (contentType != m_request->headers.end())
    {
        if (int error = parseHeaderValue(contentType->second, &RequestHandler::parseContentType))
            return error;
        if (m_request->m_HTTPFieldValue.valName == "multipart/form-data")
        {
            std::map<std::string, std::string>::const_iterator boundaryIt = m_request->m_HTTPFieldValue.parameters.find("boundary");
            if (boundaryIt == m_request->m_HTTPFieldValue.parameters.end())
                return 400;
            m_request->boundary = boundaryIt->second;
            m_request->isMultipart = true;
            m_needBody = true;
        }
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
    m_needBody = false;
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
    m_needBody = false;
}

void RequestHandler::makeUploadResponse()
{
    for (std::vector<MultipartFormData>::iterator it = m_request->m_multipartFormDatas.begin(); it != m_request->m_multipartFormDatas.end(); ++it)
    {
        std::map<std::string, std::string>::iterator filenameIt = it->dispositionParams.find("filename");
        if (filenameIt == it->dispositionParams.end() || filenameIt->second == "")
            return makeErrorResponse(400);

        NewFileResourcePtr newFileResource = NewFileResource::create(m_config.upload_path, filenameIt->second);
        if (!newFileResource)
            return makeErrorResponse(500);

        m_resources.push_back(newFileResource.dynamicCast<Resource>());
    }

    m_response->setStatusCode(201);
    m_response->body = to_vector("Created\n");
    m_response->headers["Content-Type"] = "text/plain";
    m_response->headers["Content-Length"] = to_string(m_response->body.size());
    m_needBody = false;
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

    std::vector<NewFileResourcePtr> newFileResources;
    for (std::vector<ResourcePtr>::iterator it = m_resources.begin(); it != m_resources.end(); ++it)
    {
        if((*it)->open() != 0)
        {
            if (errno == ENOENT)
                return makeErrorResponse(404);
            return makeErrorResponse(500);

            return runTasks(_this);
        }

        if (DiskResourcePtr diskResource = it->dynamicCast<DiskResource>())
        {
            if (diskResource->readFd() > 0)
                IOManager::shared().insertReadTask(new FileReadTask(diskResource, m_clientSocket, m_response));
        }

        else if (CGIProgramPtr cgiProg = it->dynamicCast<CGIProgram>())
        {
            IWriteTask* cgiWriteTask = new CGIWriteTask(cgiProg, m_request);

            IOManager::shared().insertWriteTask(cgiWriteTask);
            IOManager::shared().insertReadTask(new CGIReadTask(cgiProg, cgiWriteTask, m_clientSocket, m_response, _this));
        }

        else if (NewFileResourcePtr newFileResource = it->dynamicCast<NewFileResource>())
        {
            newFileResources.push_back(newFileResource);
        }
    }

    if (newFileResources.empty() == false)
    {
        IOManager::shared().insertWriteTask(new FileWriteTask(newFileResources, m_request, m_response, m_clientSocket));
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
        if (ResourcePtr resource = Resource::create(uri + m_location.index, m_location.root, m_location.accepted_cgi_extension))
            return internalRedirection(method, uri + m_location.index, "");

        log << "index not usable, using dir\n";
        if (method == "GET" && m_location.autoindex)
            return makeResponseAutoindex(uri);

        log << "auto index not allowed\n";

        if (m_location.root + uri == m_config.upload_path)
        {
            log << "URI is upload path";
            if (method != "POST")
                return makeErrorResponse(405);
            return makeUploadResponse();
        }

        return makeErrorResponse(403);
    }

    ResourcePtr resource = Resource::create(uri, m_location.root, m_location.accepted_cgi_extension);
    if (!resource)
    {
        if (errno == ENOENT || errno == ENOTDIR)
            return makeErrorResponse(404);
        return makeErrorResponse(500);
    }

    if (DiskResourcePtr diskResource = resource.dynamicCast<DiskResource>())
    {
        if (diskResource->isDIR())
            return makeRedirectionResponse(301, uri + '/');

        if (method == "POST") // ! Must be uppercase
            return makeErrorResponse(405);

        m_response->headers["Content-Type"] = diskResource->contentType();
        m_response->headers["Content-Length"] = to_string(diskResource->contentLength());
        m_response->body.resize(diskResource->contentLength());
        m_needBody = false;
    }

    else if (CGIProgramPtr cgiProg = resource.dynamicCast<CGIProgram>())
    {
        cgiProg->completeEnvp(*m_request, method, query, m_clientSocket, m_config);
        m_needBody = true;
    }

    m_resources = std::vector<ResourcePtr>(1, resource);
}

}
