/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/10 15:24:37 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/20 13:43:42 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTHANDLER_HPP
# define REQUESTHANDLER_HPP

#include <string>
#include <vector>

#include "Utils/Utils.hpp"
#include "HTTP/HTTPRequest.hpp"
#include "Socket/ClientSocket.hpp"
#include "RequestHandler/Resource/Resource.hpp"
#include "RequestHandler/Resource/ReadFileResource.hpp"
#include "RequestHandler/Resource/DirectoryResource.hpp"
#include "RequestHandler/Resource/CGIResource.hpp"

namespace webserv
{

struct FindValName
{
    const std::string m_valName;
    inline FindValName(const std::string& valName) : m_valName(valName) {};
    bool operator()(const HTTPFieldValue& fieldVal) const { return (fieldVal.valName == m_valName); };
};

class RequestHandler;
typedef SharedPtr<RequestHandler> RequestHandlerPtr;

class RequestHandler
{
public:
    RequestHandler(const HTTPRequestPtr& request, const ClientSocketPtr& clientSocket);

    int processRequestLine();
    int processHeaders();

    inline void makeResponse() { internalRedirection(m_request->method, m_request->uri, m_request->query); }
    void internalRedirection(const std::string& method, const std::string& uri, const std::string& query);
    void makeErrorResponse(int code);
    void makeRedirectionResponse(int code, const std::string& location);
    void makeAutoindexResponse(const std::string& uri);

    void runTasks(const RequestHandlerPtr& _this);

    bool needBody();
    bool shouldEndConnection();

private:
    int processHostHeader();
    int processConnectionHeader();
    int processContentLengthHeader();
    int processTransferEncodingHeader();
    int processContentTypeHeader();

    std::string uriTranslated(const std::string& uri);

    void runIntermediateTasks(const RequestHandlerPtr& _this);
    void runResponseTasks(const RequestHandlerPtr& _this);

    HTTPRequestPtr m_request;
    
    ClientSocketPtr m_clientSocket;

    ServerConfig m_config;
    LocationDirective m_location;

    ResourcePtr m_responseResource;
    HTTPResponsePtr m_response;

    uint32 m_internalRedirectionCount;
};

}

#endif // REQUESTHANDLER_HPP