/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Resource.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/20 16:45:29 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/08 18:28:51 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESOURCE_HPP
# define RESOURCE_HPP

#include "HTTP/HTTPRequest.hpp"
#include "Parser/ConfigParser/ConfigParser.hpp"
#include "Socket/ClientSocket.hpp"

#include <sys/stat.h>

namespace webserv
{

class Resource
{
public:
    static SharedPtr<Resource> create(const std::string& uri, const std::string& root, const std::map<std::string, std::string>& cgiExt);

    virtual int open() = 0;

    inline const std::string& path() { return m_path; }
    inline int readFd() { return m_readFd; };
    inline int writeFd() { return m_writeFd; };

    void closeReadFd();
    void closeWriteFd();

    virtual ~Resource();

protected:
    Resource(const std::string& path);

    std::string m_path;

    int m_readFd;
    int m_writeFd;
};
typedef SharedPtr<Resource> ResourcePtr;

class CGIProgram : public Resource
{
public:
    static SharedPtr<Resource> create(const std::string& program, const std::string& script, const std::map<std::string, std::string>& envp);

    int open() /*override*/;

    void completeEnvp(const HTTPRequest& request, const std::string& method, const std::string& query, const ClientSocketPtr& clientSocket, const ServerConfig& config);

    inline ~CGIProgram() /*override*/ {}

private:
    CGIProgram(const std::string& program, const std::string& script, const std::map<std::string, std::string>& envp);

    std::string m_script;
    std::map<std::string, std::string> m_envp;
};
typedef SharedPtr<CGIProgram> CGIProgramPtr;

class DiskResource : public Resource
{
public:
    static SharedPtr<Resource> create(const std::string& path);

    int open() /*override*/;

    inline const ContentType& contentType() { return m_contentType;  }
    inline uint64 contentLength() { return m_stat.st_size; }
    inline bool isDIR() { return S_ISDIR(m_stat.st_mode); };

    inline ~DiskResource() /*override*/ {}

private:
    DiskResource(const std::string& path, const struct stat& stat);

    struct stat m_stat;
    ContentType m_contentType;
};
typedef SharedPtr<DiskResource> DiskResourcePtr;

class NewFileResource : public Resource
{
public:
    static SharedPtr<NewFileResource> create(const std::string& dir, const std::string& filename);

    int open() /*override*/;

    inline ~NewFileResource() /*override*/ {}

private:
    NewFileResource(const std::string& path);

};
typedef SharedPtr<NewFileResource> NewFileResourcePtr;

}

#endif // RESOURCE_HPP