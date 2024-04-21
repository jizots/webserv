/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIResource.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/31 20:56:43 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/20 08:59:05 by tchoquet         ###   ########.fr       */
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

CGIResource::CGIResource(const std::string& program, const std::string& script, bool isBuildInCGI)
    : Resource(script.empty() ? program : script), m_program(program), m_script(script), m_isBuiltInCGI(isBuildInCGI)
{
}

int CGIResource::open()
{
    std::vector<const char *> argv;
    std::vector<const char *> envp;

    if (!m_isBuiltInCGI)
    {
        argv.push_back(m_program.c_str());
        if (!m_script.empty())
            argv.push_back(m_script.c_str());
        argv.push_back(NULL);

        envp.reserve(m_envp.size());
        for (std::map<std::string, std::string>::iterator it = m_envp.begin(); it != m_envp.end(); ++it)
            envp.push_back(strdup((it->first + "=" + it->second).c_str()));
        envp.push_back(NULL);
    }
    
    FileDescriptor toCGIfds[2] = {};
    FileDescriptor fromCGIfds[2] = {};

    if (pipeWNB(toCGIfds) < 0 || pipeRNB(fromCGIfds))
        log << "pipe(): " << std::strerror(errno) << '\n';   

    int cgiPid = fork();
    if (cgiPid < 0)
        log << "fork(): " << std::strerror(errno) << '\n';

    if (cgiPid == 0)
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
            builtinCGIUpload(m_envp);
        else
            execve(argv.front(), (char *const *)argv.data(), (char *const *)envp.data());

        std::cout << "status: 502 Bad gateway\r\n";
        std::cout << "content-type: text/plain\r\n";
        std::cout << "\r\n";
        std::cout << BUILT_IN_ERROR_PAGE(502, "Bad gateway") << std::endl;

        dup2(savedStdin, FileDescriptor::stdin()); 
        dup2(savedStdout, FileDescriptor::stdout());
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

}