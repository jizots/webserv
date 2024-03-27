/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Resource.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/13 13:26:07 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/25 19:16:36 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler/RequestHandler.hpp"

#include <unistd.h>
#include <sys/fcntl.h>
#include <cstdlib>
#include <cstring>
#include <algorithm>

namespace webserv
{

Resource::Resource(const std::string& path, const struct stat& stat)
    : m_path(path), m_stat(stat), m_readFd(-1), m_writeFd(-1)
{
}

SharedPtr<Resource> Resource::create(const std::string& uri, const std::string& root, const std::map<std::string, std::string>& cgiExt)
{
    std::string::size_type firstDot = uri.find_first_of('.');
    std::string::size_type nextSlash = firstDot == std::string::npos ? std::string::npos : uri.find_first_of('/', firstDot);
    std::string extention = firstDot == std::string::npos ? "" : uri.substr(firstDot, nextSlash == std::string::npos ? std::string::npos : nextSlash - firstDot);

    for (std::map<std::string, std::string>::const_iterator curr = cgiExt.begin(); curr != cgiExt.end(); ++curr)
    {
        if (curr->first == extention)
        {
            std::map<std::string, std::string> envp;

            if (nextSlash != std::string::npos)
                envp["PATH_INFO"] = uri.substr(nextSlash);
            
            envp["SCRIPT_NAME"] = uri.substr(0, nextSlash);
            // TODO envp["PATH_TRANSLATED"] =

            if (curr->second.size() > 0)
                return CGIProgram::create(curr->second, RMV_LAST_SLASH(root) + uri.substr(0, nextSlash), envp);
            else
                return CGIProgram::create(RMV_LAST_SLASH(root) + uri.substr(0, nextSlash), "", envp);
        }
    }

    return DiskResource::create(RMV_LAST_SLASH(root) + uri);
}

void Resource::closeReadFd()
{
    if (m_readFd > 0)
    {
        log << "Closing fd " << m_readFd << '\n';
        ::close(m_readFd);
        m_readFd = -1;
    }
}

void Resource::closeWriteFd()
{
    if (m_writeFd > 0)
    {
        log << "Closing fd " << m_writeFd << '\n';\
        ::close(m_writeFd);
        m_writeFd = -1;
    }
}

Resource::~Resource()
{
    closeReadFd();
    closeWriteFd();
}

SharedPtr<Resource> CGIProgram::create(const std::string& program, const std::string& script, const std::map<std::string, std::string>& envp)
{
    struct stat stat1;

    if (::stat(program.c_str(), &stat1) < 0)
    {
        log << "stat(): " << std::strerror(errno) << '\n';
        return NULL;
    }

    if ((stat1.st_mode & S_IXUSR) == 0)
    {
        log << "CGI program not executable\n";
        return NULL;
    }

    struct stat stat2;
    if (!script.empty() && ::stat(script.c_str(), &stat2) < 0)
    {
        log << "stat(): " << script << ": " << std::strerror(errno) << '\n';
       return NULL;
    }

    return new CGIProgram(program, stat1, script, envp);
}

void CGIProgram::completeEnvp(const HTTPRequest& request, const std::string& method, const std::string& query, const ClientSocketPtr& clientSocket, const ServerConfig& config)
{
    m_envp["CONTENT_LENGTH"] = to_string(request.contentLength);

    std::map<std::string, std::string>::const_iterator it = request.headers.find("content-type");
    if (it != request.headers.end())
        m_envp["CONTENT_TYPE"] = to_string(it->second);
    
    m_envp["QUERY_STRING"] = query;
    m_envp["REQUEST_METHOD"] = method;
    m_envp["SERVER_PROTOCOL"] = request.httpVersionStr();

    // TODO AUTH_TYPE
    // TODO REMOTE_USER

    m_envp["REMOTE_ADDR"] = clientSocket->ipAddress();
    
    // ? m_envp["REMOTE_HOST"] = clientSocket->ipAddress();
    // ? REMOTE_IDENT

    std::vector<std::string>::const_iterator it2 = std::find(config.server_names.begin(), config.server_names.end(), request.host);
    m_envp["SERVER_NAME"] = it2 != config.server_names.end() ? *it2 : config.server_names.front();

    m_envp["SERVER_PORT"] = to_string(clientSocket->masterSocket()->port());
}

int CGIProgram::open()
{
    std::vector<const char *> argv;
    std::vector<const char *> envp;

    argv.push_back(m_path.c_str());
    if (!m_script.empty())
        argv.push_back(m_script.c_str());
    argv.push_back(NULL);

    envp.reserve(m_envp.size());
    for (std::map<std::string, std::string>::iterator it = m_envp.begin(); it != m_envp.end(); ++it)
        envp.push_back(strdup((it->first + "=" + it->second).c_str()));
    envp.push_back(NULL);
    
    int toCGIfds[2];
    int fromCGIfds[2];

    if (pipe(toCGIfds) != 0)
    {
        log << "pipe(): " << std::strerror(errno) << '\n';
        return -1;
    }
    
    if (pipe(fromCGIfds) != 0)
    {
        log << "pipe(): " << std::strerror(errno) << '\n';
        close(toCGIfds[READ_END]);
        close(toCGIfds[WRITE_END]);
        return -1;
    }

    int cgiPid = fork();
    if (cgiPid < 0)
        log << "fork(): " << std::strerror(errno) << '\n';

    if (cgiPid == 0)
    {
        // CGI process (child)

        close(toCGIfds[WRITE_END]);
        close(fromCGIfds[READ_END]);

        int savedStdin;
        int savedStdout;
        if((savedStdin = dup(STDIN_FILENO)) < 0 || (savedStdout = dup(STDOUT_FILENO)) < 0)
        {
            std::string errorMsg = "dup: " + std::string(std::strerror(errno));
            ssize_t writeLen = write(fromCGIfds[WRITE_END], errorMsg.c_str(), errorMsg.size());
            (void)writeLen;
            std::exit(1);
        }

        if (dup2(toCGIfds[READ_END], STDIN_FILENO) < 0 || dup2(fromCGIfds[WRITE_END], STDOUT_FILENO) < 0)
        {
            std::string errorMsg = "dup2: " + std::string(std::strerror(errno));
            ssize_t writeLen = write(fromCGIfds[WRITE_END], errorMsg.c_str(), errorMsg.size());
            (void)writeLen;
            std::exit(1);
        }
        close(toCGIfds[READ_END]);
        close(fromCGIfds[WRITE_END]);

        execve(*argv.begin(), (char *const *)argv.data(), (char *const *)envp.data());

        std::cout << "status: 502 Bad gateway\r\n";
        std::cout << "content-type: text/plain\r\n";
        std::cout << "content-length: " << ("execve: " + std::string(std::strerror(errno))).size() << "\r\n";
        std::cout << "\r\n\r\n";
        std::cout << "execve: " << std::string(std::strerror(errno)) << std::endl;

        if (dup2(savedStdin, STDIN_FILENO) < 0 || dup2(savedStdout, STDOUT_FILENO) < 0)
            std::cout << "dup2: " << std::string(std::strerror(errno)) << std::endl;

        std::exit(1);
    }
    else
    {
        // Webserv (parent)

        for (uint64 i = 0; i < envp.size(); i++)
            std::free((void*)envp[i]);

        close(toCGIfds[READ_END]);
        close(fromCGIfds[WRITE_END]);
        
        if (cgiPid > 0)
        {
            m_writeFd = toCGIfds[WRITE_END];
            m_readFd = fromCGIfds[READ_END];

            log << "CGI process \"" << m_path << "\" created (read fd: " << m_readFd << ", write fd: " << m_writeFd << ")\n";
            return 0;
        }
        else
        {
            close(toCGIfds[WRITE_END]);
            close(fromCGIfds[READ_END]);
            return -1;
        }
    }

    return -1;
}

CGIProgram::CGIProgram(const std::string& program, const struct stat& stat, const std::string& script, const std::map<std::string, std::string>& envp)
    : Resource(program, stat), m_script(script), m_envp(envp)
{
    m_envp["GATEWAY_INTERFACE"] = "CGI/1.1";
    m_envp["SERVER_SOFTWARE"] = "webserv/1.1";
}

SharedPtr<Resource> DiskResource::create(const std::string& path)
{
    struct stat stat1;

    if (::stat(path.c_str(), &stat1) < 0)
    {
        log << "stat(): " << std::strerror(errno) << '\n';
        return NULL;
    }

    if ((stat1.st_mode & S_IRUSR) == 0)
    {
        log << path << "No read access\n";
        return NULL;
    }

    return new DiskResource(path, stat1);
}

int DiskResource::open()
{
    m_readFd = ::open(m_path.c_str(), O_RDONLY);
    if (m_readFd < 0)
    {
        log << "open(): " << std::strerror(errno) << '\n';
        return -1;
    }
    log << "File " << m_path << " opened for reading (fd: " << m_readFd << ")\n"; 
    return 0;
}

DiskResource::DiskResource(const std::string& path, const struct stat& stat)
    : Resource(path, stat), m_contentType(path)
{
}

}
