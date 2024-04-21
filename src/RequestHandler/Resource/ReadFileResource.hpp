/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReadFileResource.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/31 15:56:18 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/05 15:34:11 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef READFILERESOURCE_HPP
#define READFILERESOURCE_HPP

#include "RequestHandler/Resource/Resource.hpp"

#include <string>
#include <sys/stat.h>

#include "Utils/SharedPtr.hpp"
#include "Utils/ContentType.hpp"
#include "Utils/Types.hpp"
#include "Utils/FileDescriptor.hpp"

namespace webserv
{

class ReadFileResource : public Resource
{
public:
    ReadFileResource(const std::string& path, const struct stat& stat);

    inline const ContentType& contentType() { return m_contentType; }
    inline uint64 contentLength() { return m_stat.st_size; }
    inline const FileDescriptor& fileDescriptor() { return m_fd; }

    int open() /*override*/;

private:
    const struct stat m_stat;
    const ContentType m_contentType;
    FileDescriptor m_fd;
};
typedef SharedPtr<ReadFileResource> ReadFileResourcePtr;

} // namespace webserv

#endif // READFILERESOURCE_HPP
