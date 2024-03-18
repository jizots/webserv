/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ContentType.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchoquet <tchoquet@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 14:11:47 by tchoquet          #+#    #+#             */
/*   Updated: 2024/03/13 13:23:19 by tchoquet         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONTENTTYPE_HPP
# define CONTENTTYPE_HPP

#include <string>

namespace webserv
{

class ContentType
{
public:
    enum Id { none, html, gif, ico };

public:
    ContentType(Id id = none);
    ContentType(const std::string& ext);

private:
    Id m_id;
    std::string m_str;

public:
    ContentType& operator = (const ContentType& rhs);
    ContentType& operator = (const Id& id);
    inline bool operator == (const ContentType& rhs) const { return m_id == rhs.m_id; }
    inline operator const std::string& () const { return m_str; }
};

}

#endif // CONTENTTYPE_HPP