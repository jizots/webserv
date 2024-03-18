/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/10 15:24:37 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/13 16:57:43 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTHANDLER_HPP
# define REQUESTHANDLER_HPP

#include "Utils/Utils.hpp"
#include "ConfigParser/ConfigParser.hpp"
#include "HTTP/HTTPRequest.hpp"
#include "HTTP/HTTPResponse.hpp"
#include "Socket/ClientSocket.hpp"

#include <sys/stat.h>

namespace webserv
{

class RequestHandler
{
private:
    class Resource
    {
    public:
        Resource();
        Resource(const std::string& path);

        inline const std::string& path() { return m_path; }
        inline bool isDIR() { return S_ISDIR(m_stat.st_mode); };
        void computeIsCGI(const std::map<std::string, std::string>& cgiExt);
        inline bool isCGI() { return m_isCGI;}

        int createCGIProcess(const std::vector<std::string>& envp);
        int openReadingFile();

        inline const ContentType& contentType() {return m_contentType; }
        inline uint64 contentLength() { return m_stat.st_size; }

        inline int cgiReadFd() { return m_cgiReadFd; };
        inline int cgiWriteFd() { return m_cgiWriteFd; };
        inline int fileReadFd() { return m_fileReadFd; };
        inline int fileWriteFd() { return m_fileWriteFd; };

    private:
        std::string m_path;

        struct stat m_stat;
        std::string m_extention;
        ContentType m_contentType;
        bool m_isCGI;
        std::string m_cgiInterpreter;

        int m_cgiReadFd;
        int m_cgiWriteFd;
        int m_fileReadFd;
        int m_fileWriteFd;

    public:
        inline operator bool () { return m_path.length() > 0; };
    };
    
public:
    RequestHandler(const ClientSocketPtr& clientSocket);

    void processRequestLine(const HTTPRequest& request);
    void processHeaders(const HTTPRequest& request);
    void handleRequest(const HTTPRequest& request);

    inline bool needBody() { return m_needBody; }
    inline bool shouldEndConnection() { return m_shouldEndConnection; }

private:
    void makeResponse(const HTTPRequest& request);
    void makeResponseCode(int code, const std::map<int, std::string>& error_page);
    
    void makeEnvp(const HTTPRequest& request, std::vector<std::string>& envp);

    ClientSocketPtr m_clientSocket;
    HTTPResponsePtr m_response;

    ServerConfig m_config;
    Resource m_requestedResource;

    bool m_needBody;
    bool m_shouldEndConnection;
    uint32 m_internalRedirectionCount;
};

}

#endif // REQUESTHANDLER_HPP