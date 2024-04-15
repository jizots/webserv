/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/10 15:24:37 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/08 18:28:51 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTHANDLER_HPP
# define REQUESTHANDLER_HPP

#include "Utils/Utils.hpp"
#include "Parser/ConfigParser/ConfigParser.hpp"
#include "HTTP/HTTPRequest.hpp"
#include "Socket/ClientSocket.hpp"
#include "RequestHandler/Resource.hpp"

#include <vector>

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
    void makeErrorResponse(int code);
    void makeRedirectionResponse(int code, const std::string& location);
    void makeResponseAutoindex(const std::string& uri);
    void makeUploadResponse();

    void runTasks(const RequestHandlerPtr& _this);

    void internalRedirection(const std::string& method, const std::string& uri, const std::string& query);

    inline bool needBody() { return m_needBody; }
    inline bool shouldEndConnection() { return m_shouldEndConnection; }

private:
    HTTPRequestPtr m_request;
    static const std::string m_InvalidFieldVal;

    ClientSocketPtr m_clientSocket;

    ServerConfig m_config;
    LocationDirective m_location;

    HTTPResponsePtr m_response;
    std::vector<ResourcePtr> m_resources;

    bool m_needBody;
    bool m_shouldEndConnection;
    uint32 m_internalRedirectionCount;

private:
    int parseHeaderValue(const std::string& fieldLine, int (RequestHandler::*func)(const std::string&))
    {
        if (hasCommonCharacter(fieldLine, m_InvalidFieldVal))
        {
            log << "parseHeaderValue(): Invalid field value: " << fieldLine << "\n";
            return (400);
        }
        return ((this->*func)(fieldLine));
    }
    int parseHost(const std::string& fieldLine);
    int parseContentLength(const std::string& fieldLine);
    int parseContentType(const std::string& fieldLine);
    int parseTransferEncoding(const std::string& fieldLine);
    int parseConnection(const std::string& fieldLine) { return (parseTransferEncoding(fieldLine)); };
    HTTPFieldValue              parseSingleFieldVal(const std::string& fieldVal);
    std::vector<HTTPFieldValue> parseMultiFieldVal(const std::string& fieldVals);
};

}

#endif // REQUESTHANDLER_HPP