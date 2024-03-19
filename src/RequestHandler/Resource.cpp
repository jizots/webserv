/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Resource.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: emukamada <emukamada@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/13 13:26:07 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/19 13:56:39 by emukamada        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler/RequestHandler.hpp"

#include <unistd.h>
#include <sys/fcntl.h>
#include <cstdlib>

namespace webserv
{

RequestHandler::Resource::Resource()
    : m_cgiReadFd(-1), m_cgiWriteFd(-1),
      m_fileReadFd(-1), m_fileWriteFd(-1)
{
}

RequestHandler::Resource::Resource(const std::string& path)
    : m_cgiReadFd(-1), m_cgiWriteFd(-1),
      m_fileReadFd(-1), m_fileWriteFd(-1)
{
    if (::stat(path.c_str(), &m_stat) < 0)
    {
        log << "stat(): " << std::strerror(errno) << '\n';
        return;
    }

    m_extention = path.find_last_of('.') == std::string::npos ? "" : path.substr(path.find_last_of('.'));

    // m_contentType = ContentType(m_extention);
    m_contentType = ContentType(path);
    m_path = path;
}

void RequestHandler::Resource::computeIsCGI(const std::map<std::string, std::string>& cgiExt)
{
    if (m_extention == "")
        return (void)(m_isCGI = true);

    for (std::map<std::string, std::string>::const_iterator curr = cgiExt.begin(); curr != cgiExt.end(); ++curr)
    {
        if (curr->first == m_extention)
        {
            m_isCGI = true;
            m_cgiInterpreter = curr->second;
            return;
        }
    }
    m_isCGI = false;
}

int RequestHandler::Resource::createCGIProcess(const std::vector<std::string>& _envp)
{
    int toCGIfds[2];
    int fromCGIfds[2];

    std::vector<const char *> argv;
    std::vector<const char *> envp;

    if (m_cgiInterpreter.size() == 0)
        argv.push_back(m_path.c_str());
    else
    {
        argv.push_back(m_cgiInterpreter.c_str());
        argv.push_back(m_path.c_str());
    }
    argv.push_back(NULL);

    envp.reserve(_envp.size());
    for (unsigned long i = 0; i < _envp.size(); i++)
        envp.push_back(_envp[i].c_str());
    envp.push_back(NULL);


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

        std::cout << "execve: " << std::string(std::strerror(errno)) << std::endl;

        if (dup2(savedStdin, STDIN_FILENO) < 0 || dup2(savedStdout, STDOUT_FILENO) < 0)
            std::cout << "dup2: " << std::string(std::strerror(errno)) << std::endl;

        std::exit(1);
    }
    else
    {
        // Webserv (parent)

        close(toCGIfds[READ_END]);
        close(fromCGIfds[WRITE_END]);

        if (cgiPid > 0)
        {
            m_cgiWriteFd = toCGIfds[WRITE_END];
            m_cgiReadFd = fromCGIfds[READ_END];
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

int RequestHandler::Resource::openReadingFile()
{
    m_fileReadFd = ::open(m_path.c_str(), O_RDONLY);
    if (m_fileReadFd < 0)
    {
        log << "open(): " << std::strerror(errno) << '\n';
        return -1;
    }
    return 0;
}

}
