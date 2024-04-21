/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 18:32:54 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/20 08:58:37 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler/RequestHandler.hpp"

#include <cstdio>
#include <algorithm>

#include "IO/IOManager.hpp"
#include "IO/IOTask/WriteTask/ClientSocketWriteTask.hpp"
#include "IO/IOTask/ReadTask/FileReadTask.hpp"
#include "IO/IOTask/WriteTask/CGIWriteTask.hpp"
#include "IO/IOTask/ReadTask/CGIReadTask.hpp"
#include "Parser/HTTPHeaderValueParser/HTTPHeaderValueParser.hpp"

namespace webserv
{

RequestHandler::RequestHandler(const HTTPRequestPtr& request, const ClientSocketPtr& clientSocket)
    : m_request(request), m_clientSocket(clientSocket),
      m_config(clientSocket->masterSocket()->defaultConfig()), m_location(m_config.locations.back()),
      m_response(clientSocket->newEnqueuedResponse()),
      m_internalRedirectionCount(0)
{
}

int RequestHandler::processRequestLine()
{
    log << "processing requestLine\n";

    if (m_request->verMajor != 1 || m_request->verMinor != 1)
        return 505;

    return 0;
}

int RequestHandler::processHeaders()
{
    log << "processing headers\n";

    HTTPHeaderValueParser hvp;

    std::map<std::string, std::string>::const_iterator hostIt = m_request->headers.find("host");
    if (hostIt == m_request->headers.end())
        return 400;
    m_request->host = hvp.parseHeaderValue(hostIt->second, &HTTPHeaderValueParser::parseHost);
    if (hvp.isBadRequest())
        return 400;

    m_config = m_clientSocket->masterSocket()->configForHost(m_request->host.hostname);
    m_location = m_config.bestLocation(m_request->uri);
    
    std::map<std::string, std::string>::const_iterator connectionIt = m_request->headers.find("connection");
    if (connectionIt != m_request->headers.end())
    {
        m_request->httpFieldValues = hvp.parseHeaderValue(connectionIt->second, &HTTPHeaderValueParser::parseConnection);
        if (hvp.isBadRequest())
            return 400;
        std::vector<HTTPFieldValue>::const_iterator it = std::find_if(m_request->httpFieldValues.begin(), m_request->httpFieldValues.end(), FindValName("close"));
        if (it != m_request->httpFieldValues.end())
        {
            m_response->headers["connection"] = "close";
        }
        else
        {
            it = std::find_if(m_request->httpFieldValues.begin(), m_request->httpFieldValues.end(), FindValName("keep-alive"));
            if (it != m_request->httpFieldValues.end())
            {
                m_response->headers["connection"] = "keep-alive";
            }
            else
                return 400;
        }
    }

    std::map<std::string, std::string>::const_iterator contentLengthIt = m_request->headers.find("content-length");
    if (contentLengthIt != m_request->headers.end())
    {
        m_request->contentLength = hvp.parseHeaderValue(contentLengthIt->second, &HTTPHeaderValueParser::parseContentLength);
        if (hvp.isBadRequest())
            return 400;
    }

    std::map<std::string, std::string>::const_iterator transferEncodingIt = m_request->headers.find("transfer-encoding");
    if (transferEncodingIt != m_request->headers.end())
    {
        m_request->httpFieldValues = hvp.parseHeaderValue(transferEncodingIt->second, & HTTPHeaderValueParser::parseTransferEncoding);
        if (hvp.isBadRequest())
            return 400;
        std::vector<HTTPFieldValue>::const_iterator it = std::find_if(m_request->httpFieldValues.begin(), m_request->httpFieldValues.end(), FindValName("chunked"));
        if (it != m_request->httpFieldValues.end())
            m_request->isChunk = true;
    }

    std::map<std::string, std::string>::const_iterator contentTypeIt = m_request->headers.find("content-type");
    if (contentTypeIt != m_request->headers.end())
    {
        m_request->httpFieldValue = hvp.parseHeaderValue(contentTypeIt->second, &HTTPHeaderValueParser::parseContentType);
        if (hvp.isBadRequest())
            return 400;
        if (m_request->httpFieldValue.valName == "multipart/form-data")
        {
            std::map<std::string, std::string>::const_iterator boundaryIt = m_request->httpFieldValue.parameters.find("boundary");
            if (boundaryIt == m_request->httpFieldValue.parameters.end())
                return 400;
        }
    }

    return 0;
}

// MARK: internalRedirection
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

    if (method == "DELETE") // ! Must be uppercase
    {
        if (std::remove(uriTranslated(uri).c_str()) != 0)
        {
            if (errno == ENOENT)
                return makeErrorResponse(404);
            return makeErrorResponse(500);
        }

        return makeErrorResponse(204);
    }

    ResourcePtr resource;

    if (*(--uri.end()) == '/')
    {
        log << "Ending with '/', ";
        if (*m_location.index.begin() == '/')
            return internalRedirection(method, m_location.index, "");

        log << "checking index \"" << uriTranslated(uri) + m_location.index << "\"\n";

        if (ResourcePtr m_responseResource = Resource::create(uriTranslated(uri) + m_location.index, m_location.accepted_cgi_extension))
            return internalRedirection(method, uri + m_location.index, "");

        log << "index not usable, using \"" << uriTranslated(uri) << "\"\n";

        if (DirectoryResourcePtr directoryResource = Resource::create(uriTranslated(uri), m_location.accepted_cgi_extension).dynamicCast<DirectoryResource>())
        {
            if (method == "GET" && m_location.autoindex == true)
            {
                log << "using auto index\n";
                return makeAutoindexResponse(uri); // TODO use DirectoryResourcePtr instead of uri
            }

            log << "auto index not available\n";

            if (directoryResource->path() == m_config.upload_path)
            {
                log << "resolved URI \"" << directoryResource->path() << "\" is upload path\n";

                if (method == "POST")
                    resource = new CGIResource("Build in CGI", "", true);
                else
                    return makeErrorResponse(405);
            }
            else
                return makeErrorResponse(403);
        }
        else
        {
            if (errno == ENOENT)
                return makeErrorResponse(404);
            return makeErrorResponse(500);
        }
    }
    else
        resource = Resource::create(uriTranslated(uri), m_location.accepted_cgi_extension);
    if (resource)
    {
        if (ReadFileResourcePtr readFileResource = resource.dynamicCast<ReadFileResource>())
        {
            if (method == "POST") // ! Must be uppercase
                return makeErrorResponse(405);

            m_response->headers["Content-Type"] = readFileResource->contentType();
            m_response->headers["Content-Length"] = to_string(readFileResource->contentLength());
        }

        else if (DirectoryResourcePtr directoryResource = resource.dynamicCast<DirectoryResource>())
            return makeRedirectionResponse(301, uri + '/');

        else if (CGIResourcePtr cgiProg = resource.dynamicCast<CGIResource>())
        {
            std::string::size_type firstDot = uri.find_first_of('.');
            std::string::size_type nextSlash = firstDot == std::string::npos ? std::string::npos : uri.find_first_of('/', firstDot);
            std::string extention = firstDot == std::string::npos ? "" : uri.substr(firstDot, nextSlash == std::string::npos ? std::string::npos : nextSlash - firstDot);
            if (nextSlash != std::string::npos)
                cgiProg->setEnvp("PATH_INFO", uri.substr(nextSlash));
            cgiProg->setEnvp("SCRIPT_NAME", uri.substr(0, nextSlash));
            if (nextSlash != std::string::npos)
                cgiProg->setEnvp("PATH_TRANSLATED", uriTranslated(uri.substr(nextSlash)));
            cgiProg->setEnvp("CONTENT_LENGTH",to_string(m_request->contentLength));

            std::map<std::string, std::string>::const_iterator it = m_request->headers.find("content-type");
            if (it != m_request->headers.end())
                cgiProg->setEnvp("CONTENT_TYPE", to_string(it->second));
            
            cgiProg->setEnvp("QUERY_STRING", query);
            cgiProg->setEnvp("REQUEST_METHOD", method);
            cgiProg->setEnvp("SERVER_PROTOCOL", m_request->httpVersionStr());

            // TODO AUTH_TYPE
            // TODO REMOTE_USER

            cgiProg->setEnvp("REMOTE_ADDR", m_clientSocket->ipAddress());
            
            // ? m_envp["REMOTE_HOST"] = clientSocket->ipAddress();
            // ? REMOTE_IDENT

            std::vector<std::string>::const_iterator it2 = std::find(m_config.server_names.begin(), m_config.server_names.end(), m_request->host.hostname);
            cgiProg->setEnvp("SERVER_NAME", it2 != m_config.server_names.end() ? *it2 : m_config.server_names.front());

            cgiProg->setEnvp("SERVER_PORT", to_string(m_clientSocket->masterSocket()->port()));

            cgiProg->setEnvp("UPLOAD_PATH", m_config.upload_path);
        }

        m_responseResource = resource;
        return;
    }

    if (errno == ENOENT)
        return makeErrorResponse(404);
    return makeErrorResponse(500);
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
            IOManager::shared().insertWriteTask(new ClientSocketWriteTask(m_clientSocket, m_response));
        return;
    }

    if(m_responseResource->open() != 0)
    {
        if (errno == ENOENT)
            makeErrorResponse(404);
        else
            makeErrorResponse(500);
        return runTasks(_this);
    }

    if (ReadFileResourcePtr readFileResource = m_responseResource.dynamicCast<ReadFileResource>())
    {
        IOManager::shared().insertReadTask(new FileReadTask(readFileResource, m_response, _this));
    }

    else if (CGIResourcePtr cgiResource = m_responseResource.dynamicCast<CGIResource>())
    {
        CGIReadTask* cgiReadTask = new CGIReadTask(cgiResource->readFd(), m_response, _this);
        IOManager::shared().insertReadTask(cgiReadTask);

        if (m_request->body.empty() == false)
        {
            CGIWriteTask* cgiWriteTask = new CGIWriteTask(cgiResource->writeFd(), m_request, _this);
            IOManager::shared().insertWriteTask(cgiWriteTask);
            
            cgiReadTask->setRelatedWriteTaskPtr(cgiWriteTask);
        }
    }

    else
    {
        makeErrorResponse(500);
        return runTasks(_this);
    }

    m_responseResource.clear();
}

bool RequestHandler::needBody() 
{
    return m_responseResource.dynamicCast<CGIResource>() == true;
}

bool RequestHandler::shouldEndConnection() 
{
    if (m_response->statusCode >= 400)
        return true;

    std::map<std::string, std::string>::iterator it = m_response->headers.find("connection");
    if (it == m_response->headers.end())
        return true;

    return it->second == "close";
}

std::string RequestHandler::uriTranslated(const std::string& uri)
{
    if (m_location.alias.empty() == false)
        return m_location.alias + uri.substr(m_location.location.size());
    return RMV_LAST_SLASH(m_location.root) + uri;
}

}
