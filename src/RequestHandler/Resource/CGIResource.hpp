/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIResource.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/30 23:21:58 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/26 13:31:56 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIRESOURCE_HPP
# define CGIRESOURCE_HPP

#include "RequestHandler/Resource/Resource.hpp"

#include <string>
#include <map>

#include "Utils/SharedPtr.hpp"
#include "Utils/FileDescriptor.hpp"

namespace webserv
{

class CGIResource : public Resource
{
public:
    CGIResource(const std::string& script, const std::string& interpreter, bool isBuildInCGI = false);

    inline void setEnvp(const std::string& key, const std::string& value) { m_envp[key] = value; }

    inline const FileDescriptor& writeFd() { return m_writeFd; }
    inline const FileDescriptor& readFd() { return m_readFd; }
    inline int pid() { return m_pid; }

    int open() /*override*/;

    inline bool canExec() { return m_isBuiltInCGI || ::access((m_interpreter.empty() ? m_script : m_interpreter).c_str(), X_OK) == 0; }

private:
    const std::string m_script;
    const std::string m_interpreter;
    const bool m_isBuiltInCGI;
    std::map<std::string, std::string> m_envp;

    FileDescriptor m_writeFd;
    FileDescriptor m_readFd;
    int m_pid;
};
typedef SharedPtr<CGIResource> CGIResourcePtr;


} // namespace webserv

#endif // CGIRESOURCE_HPP