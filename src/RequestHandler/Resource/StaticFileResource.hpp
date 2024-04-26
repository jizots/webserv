/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticFileResource.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/31 15:56:18 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/23 17:09:12 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STATICFILERESOURCE_HPP
#define STATICFILERESOURCE_HPP

#include "RequestHandler/Resource/Resource.hpp"

#include <string>
#include <sys/stat.h>

#include "Utils/SharedPtr.hpp"
#include "Utils/ContentType.hpp"
#include "Utils/Types.hpp"
#include "Utils/FileDescriptor.hpp"

namespace webserv
{

class StaticFileResource : public Resource
{
public:
    StaticFileResource(const std::string& path, const struct stat& stat);

    inline const ContentType& contentType() { return m_contentType; }
    inline uint64 contentLength() { return m_stat.st_size; }
    inline const FileDescriptor& fileDescriptor() { return m_fd; }

    int open() /*override*/;

    inline bool canRead() { return ::access(m_path.c_str(), R_OK) == 0; }
    inline bool canDelete() { return ::access(m_path.substr(0, m_path.find_last_of('/')).c_str(), W_OK | X_OK) == 0; }

private:
    const struct stat m_stat;
    const ContentType m_contentType;
    FileDescriptor m_fd;
};
typedef SharedPtr<StaticFileResource> StaticFileResourcePtr;

} // namespace webserv

#endif // STATICFILERESOURCE_HPP
