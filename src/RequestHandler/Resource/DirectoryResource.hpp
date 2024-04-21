/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectoryResource.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/31 14:31:53 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/05 15:31:06 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIRECTORYRESOURCE_HPP
# define DIRECTORYRESOURCE_HPP

#include "RequestHandler/Resource/Resource.hpp"

#include <string>

#include "Utils/SharedPtr.hpp"

namespace webserv
{

class DirectoryResource : public Resource
{
public:
    DirectoryResource(const std::string& path);

    int open() /*override*/ { return 0; } // TODO
};
typedef SharedPtr<DirectoryResource> DirectoryResourcePtr;

} // namespace webserv

#endif // DIRECTORYRESOURCE_HPP