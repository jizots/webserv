/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 18:32:54 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/30 14:55:38 by tchoquet         ###   ########.fr       */
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
#include "RequestHandler/Resource/NoSuchFileResource.hpp"
#include "IO/IOTask/WriteTask/FileWriteTask.hpp"

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
    typedef int (RequestHandler::*HeaderProcessFn)();

    log << "processing headers\n";

    const HeaderProcessFn funcs[] = 
    {
        &RequestHandler::processConnectionHeader,
        &RequestHandler::processContentLengthHeader,
        &RequestHandler::processTransferEncodingHeader,
    };

    //default headers
    m_response->headers["Connection"] = "keep-alive";

    if(int error = processHostHeader())
        return error;

    m_config = m_clientSocket->masterSocket()->configForHost(m_request->hostname);
    m_location = m_config.bestLocation(m_request->uri);

    for (uint32 i = 0; i < sizeof(funcs) / sizeof(funcs[0]); i++)
    {
        if(int error = (this->*funcs[i])())
            return error;
    }

    return 0;
}

void RequestHandler::internalRedirection(std::string method, std::string uri, std::string query)
{
    if (m_internalRedirectionCount > 0)
        log << "internal redirection to uri \"" << uri << "\"\n";
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

    if (m_location.redirect.empty() == false)
    {
        if (*m_location.redirect.begin() == '/')
            return internalRedirection(method, m_location.redirect, query);
        else
            return makeRedirectionResponse(302, m_location.redirect);
    }

    m_responseResource = Resource::create(uri, m_location);

    if (!m_responseResource)
        return makeErrorResponse(403);
    
    if (!m_responseResource.dynamicCast<CGIResource>() && method != "GET" && method != "HEAD" && method != "POST" && method != "DELETE" && method != "PUT")
        return makeErrorResponse(501);
        
    if (std::find(m_location.accepted_methods.begin(), m_location.accepted_methods.end(), method) == m_location.accepted_methods.end())
        return makeErrorResponse(405);

    if (StaticFileResourcePtr staticFileResource = m_responseResource.dynamicCast<StaticFileResource>())
    {
        log << "\"" << staticFileResource->path() << "\" is resolved as static file resource\n";

        if (method == "GET" || method == "HEAD")
        {
            if (staticFileResource->canRead() == false)
            {
                log << "\"" << staticFileResource->path() << "\" cannot be read\n";
                return makeErrorResponse(403);
            }

            m_response->headers["Content-Type"] = staticFileResource->contentType();
            m_response->headers["Content-Length"] = to_string(staticFileResource->contentLength());
            
            if (method == "HEAD")
                m_response->isComplete = true;
        }

        else if (method == "POST")
            return makeErrorResponse(403); // * Forbidden

        else if (method == "DELETE")
        {
            if (staticFileResource->canDelete() == false)
            {
                log << "\"" << staticFileResource->path() << "\" cannot be delete\n";
                return makeErrorResponse(403);
            }

            if (std::remove(staticFileResource->path().c_str()) != 0)
                return makeErrorResponse(500);

            return makeErrorResponse(204);
        }

        else if (method == "PUT")
            return makeErrorResponse(403); // * Forbidden

        else
            return makeErrorResponse(501);
    }

    else if (NoSuchFileResourcePtr noSuchFileResource = m_responseResource.dynamicCast<NoSuchFileResource>())
    {
        log << "\"" << noSuchFileResource->path() << "\" is resolved as no such file resource\n";

        if (method == "GET" || method == "HEAD")
            return makeErrorResponse(404);

        else if (method == "POST")
            return makeErrorResponse(404); // * Forbidden

        else if (method == "DELETE")
            return makeErrorResponse(404);

        else if (method == "PUT")
        {
            if (m_location.client_max_body_size > 0 && m_request->contentLength > m_location.client_max_body_size)
                return makeErrorResponse(413);
            if (noSuchFileResource->canCreate() == false)
            {
                log << "\"" << noSuchFileResource->path() << "\" cannot be create\n";
                return makeErrorResponse(403); // * Forbidden
            }
        }
        else
            return makeErrorResponse(501);
    }

    else if (DirectoryResourcePtr directoryResource = m_responseResource.dynamicCast<DirectoryResource>())
    {
        log << "\"" << directoryResource->path() << "\" is resolved as directory resource\n";

        if (*(--directoryResource->path().end()) != '/')
            return makeRedirectionResponse(301, uri + '/');

        if (method == "GET" || method == "HEAD")
        {
            log << "trying index \"" << m_location.index  << "\"\n";

            ResourcePtr indexResource = Resource::create(uri + m_location.index, m_location);
            if (indexResource && !indexResource.dynamicCast<NoSuchFileResource>())
                return internalRedirection(method, uri + m_location.index, "");

             log << "index not usable\n";

            if (m_location.autoindex == true && directoryResource->canRead())
            {
                log << "using auto index\n";
                makeAutoindexResponse(uri); // TODO use DirectoryResourcePtr instead of uri
                if (m_request->method == "HEAD") // TODO not produce body when HEAD method
                    m_response->body.clear();
                return;
            }
            else
            {
                log << "auto index not available\n";
                return makeErrorResponse(403);
            }
        }

        else if (method == "POST")
        {
            if (m_location.client_max_body_size > 0 && m_request->contentLength > m_location.client_max_body_size)
                return makeErrorResponse(413);
            if (directoryResource->path() == m_config.upload_path && directoryResource->canCreateFile())
            {
                log << "resolved URI \"" << directoryResource->path() << "\" is upload path\n";
                m_responseResource = new CGIResource("Build in CGI", "", true);
                m_responseResource.dynamicCast<CGIResource>()->setEnvp("UPLOAD_PATH", m_config.upload_path);
            }
            else
                return makeErrorResponse(403);
        }

        else if (method == "DELETE")
            return makeErrorResponse(403); // * Forbidden

        else if (method == "PUT")
            return makeErrorResponse(403); // * Forbidden

        else
            return makeErrorResponse(501);
    }

    if (CGIResourcePtr cgiProg = m_responseResource.dynamicCast<CGIResource>())
    {
        log << "\"" << cgiProg->path() << "\" is resolved as cgi resource\n";

        if (m_location.client_max_body_size > 0 && m_request->contentLength > m_location.client_max_body_size)
            return makeErrorResponse(413);

        if (cgiProg->canExec() == false)
        {
            log << "\"" << cgiProg->path() << "\" is not executable\n";
            return makeErrorResponse(403); // * Forbidden
        }
        
        cgiProg->setEnvp("CONTENT_LENGTH", to_string(m_request->contentLength));
        if (query.empty() == false)
            cgiProg->setEnvp("QUERY_STRING", query);
        cgiProg->setEnvp("REQUEST_METHOD", method);
        cgiProg->setEnvp("SERVER_PROTOCOL", m_request->httpVersionStr());
        cgiProg->setEnvp("REMOTE_ADDR", m_clientSocket->ipAddress());
        cgiProg->setEnvp("SERVER_PORT", to_string(m_clientSocket->masterSocket()->port()));
        
        std::map<std::string, std::string>::const_iterator it = m_request->headers.find("content-type");
        if (it != m_request->headers.end())
            cgiProg->setEnvp("CONTENT_TYPE", to_string(it->second));

        it = m_request->headers.find("cookie");
        if (it != m_request->headers.end())
            cgiProg->setEnvp("HTTP_COOKIE", to_string(it->second));

        std::vector<std::string>::const_iterator it2 = std::find(m_config.server_names.begin(), m_config.server_names.end(), m_request->hostname);
        cgiProg->setEnvp("SERVER_NAME", it2 != m_config.server_names.end() ? *it2 : m_config.server_names.front());

        // TODO `AUTH_TYPE` 'auth-scheme' token in the request Authorization header field
        // TODO `REMOTE_USER` derive from AUTH_TYPE
    }
}

void RequestHandler::makeErrorResponse(int code)
{
    log << "error code " << code << '\n';

    m_response->headers.erase("location");

    if (code == 405)
    {
        for (std::vector<std::string>::iterator it = m_location.accepted_methods.begin(); it != m_location.accepted_methods.end(); ++it)
        {
            if (it == m_location.accepted_methods.begin())
                m_response->headers["Allow"] = *it;
            else
                m_response->headers["Allow"] += ", " + *it;
        }
    }
    else
        m_response->headers.erase("Allow");

    if (m_response->statusCode == 200)
    {
        std::map<int, std::string>::const_iterator userPage = m_location.error_page.find(code);

        if (userPage != m_location.error_page.end() && *userPage->second.begin() != '/')
            return makeRedirectionResponse(302, userPage->second);

        m_response->setStatusCode(code);

        if (userPage != m_location.error_page.end() && *userPage->second.begin() == '/')
            return internalRedirection(m_request->method == "HEAD" ? "HEAD" : "GET", std::string(userPage->second), "");
    }

    m_responseResource.clear();
    m_response->makeBuiltInResponse(code);
    if (m_request->method == "HEAD")
        m_response->body.clear();
}

void RequestHandler::makeRedirectionResponse(int code, const std::string& location)
{
    log << "redirection code " << code << " to \"" << location << "\"\n";

    m_response->headers["location"] = location;
    m_response->headers.erase("Allow");

    if (m_response->statusCode == 200)
    {
        std::map<int, std::string>::const_iterator userPage = m_location.error_page.find(code);

        m_response->setStatusCode(code);

        if (userPage != m_location.error_page.end() && *userPage->second.begin() != '/')
            return makeRedirectionResponse(302, userPage->second);

        if (userPage != m_location.error_page.end() && *userPage->second.begin() == '/')
            return internalRedirection(m_request->method == "HEAD" ? "HEAD" : "GET", std::string(userPage->second), "");
    }

    m_responseResource.clear();
    m_response->makeBuiltInResponse(code);
    if (m_request->method == "HEAD")
        m_response->body.clear();
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

    if (StaticFileResourcePtr staticFileResource = m_responseResource.dynamicCast<StaticFileResource>())
    {
        IOManager::shared().insertReadTask(new FileReadTask(staticFileResource, m_response, _this));
    }

    else if (NoSuchFileResourcePtr noSuchFileResource = m_responseResource.dynamicCast<NoSuchFileResource>())
    {
        IOManager::shared().insertWriteTask(new FileWriteTask(noSuchFileResource, m_request, _this));
    }

    else if (CGIResourcePtr cgiResource = m_responseResource.dynamicCast<CGIResource>())
    {
        CGIReadTask* cgiReadTask = new CGIReadTask(cgiResource->readFd(), cgiResource->pid(), m_response, _this);
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
    if (m_responseResource.dynamicCast<CGIResource>() == true)
        return true;
    if (m_responseResource.dynamicCast<NoSuchFileResource>() == true)
        return true;
    return false;
}

bool RequestHandler::shouldEndConnection() 
{
    if (m_response->statusCode >= 400)
        return true;

    std::map<std::string, std::string>::iterator it = m_response->headers.find("Connection");
    if (it == m_response->headers.end())
    {
        log << "Connection header field not present in response\n";
        return false;
    }

    return it->second == "close";
}

}
