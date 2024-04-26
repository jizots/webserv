/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NoSuchFileResource.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/21 16:54:38 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/23 17:08:30 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NOSUCHFILERESOURCE_HPP
# define NOSUCHFILERESOURCE_HPP

#include "RequestHandler/Resource/Resource.hpp"

#include "Utils/FileDescriptor.hpp"

namespace webserv
{

class NoSuchFileResource : public Resource
{
public:
    NoSuchFileResource(const std::string& path);

    inline const FileDescriptor& fileDescriptor() { return m_fd; }
    
    int open() /*override*/;

    inline bool canCreate() { return ::access(m_path.substr(0, m_path.find_last_of('/')).c_str(), W_OK | X_OK) == 0; }


private:
    FileDescriptor m_fd;
};
typedef SharedPtr<NoSuchFileResource> NoSuchFileResourcePtr;

}

#endif // NOSUCHFILERESOURCE_HPP