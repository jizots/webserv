/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ContentType.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:12:54 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/09 14:17:32 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/ContentType.hpp"

namespace webserv
{

ContentType::ContentType(Id id_) : m_id(none)
{
    *this = id_;
}

ContentType::ContentType(const std::string& ext) : m_id(none)
{
    if (ext == ".html")
        *this = html;
    else if (ext == ".gif")
        *this = gif;
    else if (ext == ".ico")
        *this = ico;
}

ContentType& ContentType::operator = (const ContentType& rhs)
{
    if (this != &rhs)
    {
        m_id = rhs.m_id;
        m_str = rhs.m_str;
    }
    return *this;
}

ContentType& ContentType::operator = (const Id& id)
{
    if (m_id != id)
    {
        m_id = id;
        switch (m_id)
        {
        case html:
            m_str = "text/html";
            break;

        case gif:
            m_str = "image/gif";
            break;

        case ico:
            m_str = "image/vnd.microsoft.icon";
            break;

        default:
            m_str = "";
            break;
        }
    }
    return *this;
}

}