/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectoryResource.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/31 18:46:55 by tchoquet          #+#    #+#             */
/*   Updated: 2024/04/01 12:39:51 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestHandler/Resource/DirectoryResource.hpp"

#include <string>

namespace webserv
{

DirectoryResource::DirectoryResource(const std::string& path)
    : Resource(path)
{
}

}