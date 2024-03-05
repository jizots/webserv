/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTP.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/18 18:47:29 by tchoquet          #+#    #+#             */
/*   Updated: 2024/02/19 13:48:38 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_HPP
# define HTTP_HPP

#include <map>
#include <string>
#include <vector>

#include "Utils/Utils.hpp"

namespace webserv
{

struct HTTP
{
    enum ContentType {none, html, gif, icon };

    inline HTTP(uint8 versionMajor = 1, uint8 versionMinor = 1) : httpVersionMajor(versionMajor), httpVersionMinor(versionMinor) {}

    uint8 httpVersionMajor;
    uint8 httpVersionMinor;
    std::map<std::string, std::string> headers;
    std::vector<Byte> body;

    inline std::string httpVersionStr() const { return "HTTP/" + to_string((int)httpVersionMajor) + '.' + to_string((int)httpVersionMinor); }
    
    inline std::string& operator [] (const std::string& key) { return headers[key]; }
};

struct HTTPRequest : public HTTP
{
    inline HTTPRequest()
        : HTTP(), requestedFile("/"), requestedServerHostName("localhost"), isBadRequest(false) {};

    inline HTTPRequest(const HTTPRequest& cp, const std::string& method, const std::string& requestedFile)
        : HTTP(cp), method(method), requestedFile(requestedFile), requestedServerHostName(cp.requestedServerHostName), isBadRequest(false) {};

    std::string method;
    std::string requestedFile;
    std::string requestedServerHostName;
    bool isBadRequest;
};

struct HTTPResponse : public HTTP
{
    HTTPResponse(int code = 200);
    
    uint32 statusCode;
    std::string statusDescription;
    bool isComplete;
    ContentType contentType;

    inline std::string firstLine() const { return httpVersionStr() + ' ' + to_string(statusCode) + ' ' + statusDescription; }

    void setStatusCode(uint32 code);
    void setContentType(const std::string& filePath);
    void setBody(const std::string& str);
    void completeResponse();
    void getRaw(std::vector<Byte>&) const;
};

typedef SharedPtr<HTTPResponse> HTTPResponsePtr;

}

#endif // HTTP_HPP
