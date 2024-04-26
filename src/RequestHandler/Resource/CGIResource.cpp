/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIResource.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/31 20:56:43 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/26 13:10:39 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler/Resource/CGIResource.hpp"

#include <unistd.h>
#include <sys/unistd.h>
#include <vector>
#include <cstdlib>

#include "Utils/Macros.hpp"
#include "Utils/Logger.hpp"
#include "RequestHandler/BuiltinCGIUpload.hpp"
#include "HTTP/HTTPResponse.hpp"

namespace webserv
{

CGIResource::CGIResource(const std::string& script, const std::string& interpreter, bool isBuildInCGI)
    : Resource(script), m_script(script), m_interpreter(interpreter), m_isBuiltInCGI(isBuildInCGI)
{
}

int CGIResource::open()
{    
    FileDescriptor toCGIfds[2] = {};
    FileDescriptor fromCGIfds[2] = {};

    if (pipeWNB(toCGIfds) < 0 || pipeRNB(fromCGIfds))
        log << "pipe(): " << std::strerror(errno) << '\n';   

    m_pid = fork();
    if (m_pid < 0)
        log << "fork(): " << std::strerror(errno) << '\n';

    if (m_pid == 0)
    {
        // CGI process (child)
        Logger::shared().setIsChildProcess(true);

        close(toCGIfds[WRITE_END]);
        close(fromCGIfds[READ_END]);

        FileDescriptor savedStdin = dup(FileDescriptor::stdin());
        FileDescriptor savedStdout = dup(FileDescriptor::stdout());
        if(!savedStdin || !savedStdout)
        {
            write(fromCGIfds[WRITE_END], "dup: " + std::string(std::strerror(errno)));
            std::exit(1);
        }

        if (!dup2(toCGIfds[READ_END], FileDescriptor::stdin()) || !dup2(fromCGIfds[WRITE_END], FileDescriptor::stdout()))
        {
            write(fromCGIfds[WRITE_END], "dup2: " + std::string(std::strerror(errno)));
            std::exit(1);
        }
        close(toCGIfds[READ_END]);
        close(fromCGIfds[WRITE_END]);

        if (m_isBuiltInCGI)
        {
            builtinCGIUpload(m_envp);
            std::cout << "status: 502 Bad gateway\r\n";
            std::cout << "content-type: text/html\r\n";
#ifndef NDEBUG
            std::cout << "Error: " << "builtinCGIUpload(): unexpected return" << std::endl;
#endif // NDEBUG
            std::cout << "\r\n";
            std::cout << BUILT_IN_ERROR_PAGE(502, "Bad gateway") << std::endl;
        }
        else
        {
            if (chdir(m_script.substr(0, m_script.find_last_of('/')).c_str()) == 0)
            {
                std::vector<const char *> argv;
                std::vector<const char *> envp;

                if (m_interpreter.empty() == false)
                    argv.push_back(m_interpreter.c_str());
                argv.push_back(m_script.substr(m_script.find_last_of('/') + 1).c_str());
                argv.push_back(NULL);

                envp.reserve(m_envp.size());
                for (std::map<std::string, std::string>::iterator it = m_envp.begin(); it != m_envp.end(); ++it)
                    envp.push_back(strdup((it->first + "=" + it->second).c_str()));
                envp.push_back(NULL);
                
                execve(argv.front(), (char *const *)argv.data(), (char *const *)envp.data());
            } 
            std::cout << "status: 502 Bad gateway\r\n";
            std::cout << "content-type: text/html\r\n";
#ifndef NDEBUG
            std::cout << "errno: " << std::string(std::strerror(errno)) <<"\r\n";
#endif // NDEBUG
            std::cout << "\r\n";
            std::cout << BUILT_IN_ERROR_PAGE(502, "Bad gateway") << std::endl;
        }

        dup2(savedStdin, FileDescriptor::stdin()); 
        dup2(savedStdout, FileDescriptor::stdout());
        std::exit(1);
    }
    else
    {
        // Webserv (parent)

        close(toCGIfds[READ_END]);
        close(fromCGIfds[WRITE_END]);
        
        if (m_pid > 0)
        {
            m_writeFd = toCGIfds[WRITE_END];
            m_readFd = fromCGIfds[READ_END];

            log << "CGI process for script \"" << m_path << "\" created (read fd: " << m_readFd << ", write fd: " << m_writeFd << ")\n";
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

}