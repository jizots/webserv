/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectoryResource.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/31 14:31:53 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/23 16:58:34 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIRECTORYRESOURCE_HPP
# define DIRECTORYRESOURCE_HPP

#include "RequestHandler/Resource/Resource.hpp"

#include <string>
#include <unistd.h>

#include "Utils/SharedPtr.hpp"

namespace webserv
{

class DirectoryResource : public Resource
{
public:
    DirectoryResource(const std::string& path);

    int open() /*override*/ { return 0; } // TODO

    inline bool canRead() { return ::access(m_path.c_str(), R_OK) == 0; }
    inline bool canCreateFile() { return ::access(m_path.c_str(), W_OK | X_OK) == 0; }

};
typedef SharedPtr<DirectoryResource> DirectoryResourcePtr;

} // namespace webserv

#endif // DIRECTORYRESOURCE_HPP